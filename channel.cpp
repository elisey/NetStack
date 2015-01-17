#include "channel.h"
#include "debug.h"

void Channel_TxTask(void *param);

Channel::Channel()
{
	txQueue = xQueueCreate(10, sizeof(Frame));
	rxQueue = xQueueCreate(10, sizeof(Frame));

	xTaskCreate(
			Channel_TxTask,
			"Channel_TxTask",
			200,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

void Channel::send(const Frame *ptrFrame)
{
	BaseType_t result;
	result = xQueueSend(txQueue, ptrFrame, (TickType_t)portMAX_DELAY);
	assert(result == pdPASS);
}

bool Channel::receive(Frame *ptrFrame, unsigned int timeout)
{
	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrFrame, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

void Channel_TxTask(void *param)
{
	Channel *ptrObj = static_cast<Channel*>(param);
	ptrObj->txTask();
	while(1);
}

void Channel::txTask()
{
	while(1)
	{
		Frame frame;
		BaseType_t result;
		result = xQueueReceive(txQueue, &(frame), (portMAX_DELAY));
		assert(result == pdPASS);

		transfer(&frame);

		while(isTxBusy() == true)	{
			vTaskDelay(1 / portTICK_RATE_MS);
		}
		//frame.free();
	}
}

void Channel::handleRxPacket(Frame *ptrFrame)
{
	BaseType_t higherPriorityTaskWoken;
	BaseType_t result;
	result = xQueueSendFromISR(rxQueue, ptrFrame, &higherPriorityTaskWoken);
	assert(result == pdPASS);
	if (higherPriorityTaskWoken == pdTRUE)	{
		taskYIELD();
	}
}
