#include "mac_layer.h"

static void MacLayer_RxTask(void *param);

MacLayer::MacLayer(Channel* _ptrChannel)
	:	ptrChannel(_ptrChannel), nextPid(0)
{
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

bool MacLayer::send(PoolNode *ptrPoolNode, uint16_t dstAddress)
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

bool MacLayer::receive(PoolNode *ptrPoolNode, unsigned int timeout)
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
	else {
		transferOk = true;
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

	BaseType_t result = xSemaphoreTake(txMutex, 50);
	if (result == pdPASS)	{
		transfer(macFrame);
		xSemaphoreGive(txMutex);
	}
	else	{
		macFrame.free();
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
	result = xQueueReceive(ackQueue, &receivedPid, (TickType_t)2 / portTICK_RATE_MS);
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
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)50);
	assert(result == pdPASS);
	//ptrMacFrame->free();
}

uint8_t MacLayer::getUniquePid()
{
	return nextPid++;
}
