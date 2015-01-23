#include "mac_layer.h"

static void MacLayer_TxTask(void *param);
static void MacLayer_RxTask(void *param);

MacLayer::MacLayer(Channel* _ptrChannel)
{
	ptrChannel = _ptrChannel;

	txQueue = xQueueCreate(10, sizeof(MacFrame));
	rxQueue = xQueueCreate(10, sizeof(MacFrame));
	ackQueue = xQueueCreate(3, sizeof(uint8_t));

	xTaskCreate(
			MacLayer_TxTask,
			"MacLayer_TxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
	xTaskCreate(
			MacLayer_RxTask,
			"MacLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 2,
			NULL);
}

static void MacLayer_TxTask(void *param)
{
	MacLayer *ptrObj = static_cast<MacLayer*>(param);
	ptrObj->txTask();
	while(1);
}

void MacLayer::txTask()
{
	while(1)
	{
		MacFrame macFrame;
		BaseType_t result;
		result = xQueueReceive(txQueue, &macFrame, portMAX_DELAY);
		assert(result == pdTRUE);

		packetAckType_t ackType = macFrame.getPacketAckType();
		uint8_t packetPid = macFrame.getPid();
		Frame frame;
		frame.clone(macFrame);

		clearQueueAck();
		ptrChannel->send(&frame);

		if (ackType == packetAckType_withAck)	{

			unsigned int i;
			for (i = 0; i < mac_layerRESEND_NUM - 1; ++i) {
				if (isAckReceived(packetPid) == true)	{
					break;
				}
				clearQueueAck();
				ptrChannel->send(&frame);
			}
		}
		frame.free();
	}
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



void MacLayer::send(MacFrame *ptrMacFrame, packetAckType_t packetAckType)
{
	ptrMacFrame->setPacketAckType(packetAckType);
	ptrMacFrame->setPid( getUniquePid() );
	// добавить место для CRC в буфер
	ptrMacFrame->getBuffer().setLenght( ptrMacFrame->getBuffer().getLenght() + mac_layerCRC_SIZE );
	ptrMacFrame->calculateAndSetCrc();
	transfer(ptrMacFrame);
}

bool MacLayer::receive(MacFrame* ptrMacFrame, unsigned int timeout)
{
	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrMacFrame, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

void MacLayer::transfer(MacFrame* ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(txQueue, ptrMacFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
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

	transfer(&macFrame);
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
/*	BaseType_t result;
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);*/
	ptrMacFrame->free();
}

uint8_t MacLayer::getUniquePid()
{
	static uint8_t pid = 0;
	return pid++;
}
