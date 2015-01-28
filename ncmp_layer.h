#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "NpFrame.h"

class NcmpLayer
{
public:
	NcmpLayer(uint8_t _interfaceId);
	void task();
private:
	void taskMainInterface();
	void taskSlaveInterface();



	void sendPong(uint16_t dstAddress);
	void sendPong(uint16_t dstAddress);
	void sendImHere();
	void sendRt(uint16_t dstAddress);

	uint8_t interfaceId;
	QueueHandle_t rxQueue;
	NpLayer *ptrNpLayer;
};
