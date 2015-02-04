#include "mac_layer.h"

static void MacLayer_RxTask(void *param);

MacLayer::MacLayer(Channel* _ptrChannel)
	:	ptrChannel(_ptrChannel)
{
	//ptrChannel = _ptrChannel;

	rxQueue = xQueueCreate(10, sizeof(MacFrame));
	ackQueue = xQueueCreate(3, sizeof(uint8_t));
	txMutex = xSemaphoreCreateMutex();
	// Реализовать класс LC , который будет параллельно принимать и отправлять пинги.
	// На мак уровне должно быть уже известно тип интерфейса: мастер или слейв

	xTaskCreate(
			MacLayer_RxTask,
			"MacLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 2,
			NULL);
}

static void MacLayer_RxTask(void *param)
{
	MacLayer *ptrObj = static_cast<MacLayer*>(param);
	ptrObj->rxTask();
	while(1);
}

void MacLayer::rxTask()
{
	while(1)
	{
		Frame frame;
		bool result;
		result = ptrChannel->receive(&frame, portMAX_DELAY);
		assert(result == true);

		MacFrame macFrame;
		macFrame.clone(frame);

		if (macFrame.checkCrc() == false)	{
			macFrame.free();
			continue;
		}

		//Убрать CRC из буфера.
		macFrame.getBuffer().setLenght( macFrame.getBuffer().getLenght() - mac_layerCRC_SIZE );

		packetAckType_t ackType = macFrame.getPacketAckType();
		uint8_t pid = macFrame.getPid();

		// Для пакетов Ack не учитываем их уникальность, обрабатываем всегда.
		// Остальные фильтруем
		if (ackType != packetAckType_Ack)	{
			if (uniqueFrame.isFrameUnique(pid) == false)	{
				macFrame.free();
				continue;
			}
			else	{
				uniqueFrame.putNewFrame(pid);
			}
		}

		switch(ackType)
		{
		case packetAckType_Ack:
			ackReceived(macFrame.getPid());
			macFrame.free();
			break;

		case packetAckType_noAck:
			handleRxPacket(&macFrame);
			break;
		case packetAckType_withAck:
			sendAck(macFrame.getPid());
			handleRxPacket(&macFrame);
			break;
		default:
			macFrame.free();
			break;
		}
	}
}

bool MacLayer::send(MacFrame &macFrame, packetAckType_t packetAckType)
{
	macFrame.setPacketAckType(packetAckType);
	macFrame.setPid( getUniquePid() );
	// добавить место для CRC в буфер
	macFrame.getBuffer().setLenght( macFrame.getBuffer().getLenght() + mac_layerCRC_SIZE );
	macFrame.calculateAndSetCrc();

	bool result;

	xSemaphoreTake(txMutex, portMAX_DELAY);
	result = transfer(macFrame);
	xSemaphoreGive(txMutex);

	return result;
}

bool MacLayer::receive(MacFrame &macFrame, unsigned int timeout)
{
	BaseType_t result;
	result = xQueueReceive(rxQueue, &macFrame, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

bool MacLayer::transfer(MacFrame &macFrame)
{
	packetAckType_t ackType = macFrame.getPacketAckType();
	uint8_t packetPid = macFrame.getPid();
	Frame frame;
	frame.clone(macFrame);

	clearQueueAck();

	ptrChannel->send(&frame);
	bool transferOk = false;
	if (ackType == packetAckType_withAck)	{

		unsigned int i;
		for (i = 0; i < mac_layerRESEND_NUM - 1; ++i) {
			if (isAckReceived(packetPid) == true)	{
				transferOk = true;
				break;
			}
			clearQueueAck();
			ptrChannel->send(&frame);
		}
		transferOk = false;
	}
	frame.free();
	return transferOk;
}

void MacLayer::sendAck(uint8_t pid)
{
	MacFrame macFrame;
	if (macFrame.alloc(0) == false	)	{
		return;
	}
	macFrame.getBuffer().setLenght(4);
	macFrame.setPid(pid);
	macFrame.setPacketAckType(packetAckType_Ack);
	macFrame.calculateAndSetCrc();

	BaseType_t result = xSemaphoreTake(txMutex, 3);
	if (result == pdPASS)	{
		transfer(macFrame);
		xSemaphoreGive(txMutex);
	}


}

void MacLayer::ackReceived(uint8_t pid)
{
	BaseType_t result;
	result = xQueueSend(ackQueue, &pid, (TickType_t)1 / portTICK_RATE_MS);
	if (result != pdPASS)	{
		//учет ошибки
	}
}

bool MacLayer::isAckReceived(uint8_t pid)
{
	uint8_t receivedPid;
	BaseType_t result;
	result = xQueueReceive(ackQueue, &receivedPid, (TickType_t)1 / portTICK_RATE_MS);
	if (result == pdPASS)	{
		return (receivedPid == pid);
	}
	return false;
}

void MacLayer::clearQueueAck()
{
	bool result = pdPASS;
	uint8_t dummyPid = 0;

	do	{
		result = xQueueReceive(ackQueue, &dummyPid, (TickType_t)0);
	} while(result == pdPASS);
}

void MacLayer::handleRxPacket(MacFrame *ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
	//ptrMacFrame->free();
}

uint8_t MacLayer::getUniquePid()
{
	static uint8_t pid = 0;
	return pid++;
}
