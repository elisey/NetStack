#include "channel.h"
#include "debug.h"

Channel::Channel()
{
	rxQueue = xQueueCreate(20, sizeof(Frame));
}

void Channel::send(Frame *ptrFrame)
{
	transfer(ptrFrame);
	waitForTransferCompleate();
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

void Channel::handleRxPacket(Frame *ptrFrame)
{
	BaseType_t higherPriorityTaskWoken;
	BaseType_t result;
	result = xQueueSendFromISR(rxQueue, ptrFrame, &higherPriorityTaskWoken);
	assert(result == pdPASS);		//TODO Drop if full
	if (higherPriorityTaskWoken == pdTRUE)	{
		taskYIELD();
	}
}
