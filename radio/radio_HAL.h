#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "RadioMacFrame.h"
#include "mac_layer_base.h"

class RadioHAL : public MacLayerBase
{
public:
	RadioHAL();

	void rxTask();
	bool send( PoolNode *ptrPoolNode, uint16_t dstAddress);
	bool receive(PoolNode *ptrPoolNode, unsigned int timeout);
	void init(uint16_t selfAddress);
private:


	bool transfer(uint8_t *buffer, uint8_t size, uint16_t dstAddress);
	bool transferBroadcast(uint8_t *buffer, uint8_t size);
	void wordToBuffer(uint16_t inputData, uint8_t *buffer);

	QueueHandle_t rxQueue;
	SemaphoreHandle_t nrfMutex;
};
