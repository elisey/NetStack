#include "MacLayerSFBus.h"

static void MacLayer_RxTask(void *param);

MacLayerSFBus::MacLayerSFBus(Channel* _ptrChannel)
	:	ptrChannel(_ptrChannel), nextPid(0)
{
	setMaxPayloadSize(mac_layerMAX_PAYLOAD_SIZE);

	rxQueue = xQueueCreate(10, sizeof(MacFrame));
	ackQueue = xQueueCreate(3, sizeof(uint8_t));
	txMutex = xSemaphoreCreateMutex();

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
	MacLayerSFBus *ptrObj = static_cast<MacLayerSFBus*>(param);
	ptrObj->rxTask();
	while(1);
}

void MacLayerSFBus::rxTask()
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

bool MacLayerSFBus::send(PoolNode *ptrPoolNode, uint16_t dstAddress)
{
	MacFrame macFrame;
	macFrame.clone(*ptrPoolNode);

	macFrame.setPacketAckType(packetAckType_withAck);
	macFrame.setPid( getUniquePid() );
	// добавить место для CRC в буфер
	macFrame.getBuffer().setLenght( macFrame.getBuffer().getLenght() + mac_layerCRC_SIZE );
	macFrame.calculateAndSetCrc();

	bool result;
	BaseType_t mutexTakeResult;

	mutexTakeResult = xSemaphoreTake(txMutex, portMAX_DELAY);
	assert(mutexTakeResult == pdPASS);
	result = transfer(macFrame);
	xSemaphoreGive(txMutex);

	return result;
}

bool MacLayerSFBus::receive(PoolNode *ptrPoolNode, unsigned int timeout)
{
/*	MacFrame macFrame;
	macFrame.clone(*ptrPoolNode);*/

	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrPoolNode, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

bool MacLayerSFBus::transfer(MacFrame &macFrame)
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
	else {
		transferOk = true;
	}
	frame.free();
	return transferOk;
}

void MacLayerSFBus::sendAck(uint8_t pid)
{
	MacFrame macFrame;
	if (macFrame.alloc(0) == false	)	{
		return;

	}
	macFrame.getBuffer().setLenght(4);
	macFrame.setPid(pid);
	macFrame.setPacketAckType(packetAckType_Ack);
	macFrame.calculateAndSetCrc();

	BaseType_t result = xSemaphoreTake(txMutex, 50);
	if (result == pdPASS)	{
		transfer(macFrame);
		xSemaphoreGive(txMutex);
	}
	else	{
		macFrame.free();
	}
}

void MacLayerSFBus::ackReceived(uint8_t pid)
{
	BaseType_t result;
	result = xQueueSend(ackQueue, &pid, (TickType_t)1 / portTICK_RATE_MS);
	if (result != pdPASS)	{
		//учет ошибки
	}
}

bool MacLayerSFBus::isAckReceived(uint8_t pid)
{
	uint8_t receivedPid;
	BaseType_t result;
	result = xQueueReceive(ackQueue, &receivedPid, (TickType_t)mac_layerWAIT_ACK_TIMEOUT / portTICK_RATE_MS);
	if (result == pdPASS)	{
		return (receivedPid == pid);
	}
	return false;
}

void MacLayerSFBus::clearQueueAck()
{
	bool result = pdPASS;
	uint8_t dummyPid = 0;

	do	{
		result = xQueueReceive(ackQueue, &dummyPid, (TickType_t)0);
	} while(result == pdPASS);
}

void MacLayerSFBus::handleRxPacket(MacFrame *ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)50);
	assert(result == pdPASS);
	//ptrMacFrame->free();
}

uint8_t MacLayerSFBus::getUniquePid()
{
	return nextPid++;
}
