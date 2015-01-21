#include "mac_layer.h"

static void MacLayer_TxTask(void *param);
static void MacLayer_RxTask(void *param);

MacLayer::MacLayer(Channel* _ptrChannel)
{
	txQueue = xQueueCreate(10, sizeof(MacFrame));
	rxQueue = xQueueCreate(10, sizeof(MacFrame));
	ackQueue = xQueueCreate(1, sizeof(uint8_t));

	xTaskCreate(
			MacLayer_TxTask,
			"MacLayer_TxTask",
			200,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
	xTaskCreate(
			MacLayer_RxTask,
			"MacLayer_RxTask",
			200,
			this,
			tskIDLE_PRIORITY + 1,
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


		ptrChannel->send(&frame);
		while(ptrChannel->isTxBusy() == true)	{
			vTaskDelay(1 / portTICK_RATE_MS);
		}

		if (ackType == packetAckType_withAck)	{

			unsigned int i;
			for (i = 0; i < 3; ++i) {
				if (isAckReceived(packetPid) == true)	{
					break;
				}
				ptrChannel->send(&frame);
				while(ptrChannel->isTxBusy() == true)	{
					vTaskDelay(1 / portTICK_RATE_MS);
				}
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

		//Проверкаа на уникальность (только не АК)

		if (macFrame.checkCrc() == false)	{
			macFrame.free();
			return;
		}

		switch(macFrame.getPacketAckType())
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
		}
	}
}

void MacLayer::send(MacFrame* ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(txQueue, ptrMacFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
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

void MacLayer::sendAck(uint8_t pid)
{
	MacFrame macFrame;
	macFrame.alloc();
	macFrame.getBuffer().setLenght(4);
	macFrame.setPid(pid);
	macFrame.setPacketAckType(packetAckType_Ack);
	uint16_t crc = macFrame.calculateCrc();
	macFrame.setCrc( crc );

	send(&macFrame);
}

void MacLayer::ackReceived(uint8_t pid)
{
	BaseType_t result;
	result = xQueueSend(ackQueue, &pid, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
}

bool MacLayer::isAckReceived(uint8_t pid)
{
	uint8_t receivedPid;
	BaseType_t result;
	result = xQueueReceive(ackQueue, &receivedPid, 2 / portTICK_RATE_MS);
	if (result == pdPASS)	{
		return (receivedPid == pid);
	}
	return false;
}

void MacLayer::handleRxPacket(MacFrame *ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
}
