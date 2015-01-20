#include "mac_layer.h"

static void MacLayer_Task();

MacLayer::MacLayer(Channel* _ptrChannel)
{
	txQueue = xQueueCreate(10, sizeof(MacFrame));
	rxQueue = xQueueCreate(10, sizeof(MacFrame));

	xTaskCreate(
			MacLayer_Task,
			"MacLayer_Task",
			200,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);

}

static void MacLayer_Task()
{
	MacLayer *ptrObj = static_cast<MacLayer*>(param);
	ptrObj->task();
	while(1);
}

void MacLayer::task()
{
	while(1)
	{

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

void MacLayer::handleRxPacket(MacFrame *ptrMacFrame)
{

}

void MacLayer::sendAck(uint8_t pid)
{
	//Сформировать фрейм и отправить.
	//ptrChannel->send()
}

