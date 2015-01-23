#pragma once

#include <stdint.h>
#include "NpFrame.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define selfAddress 0x33

class NpLayer
{
public:
	NpLayer(MacLayer *_ptrMacLayer, uint16_t _maxMtu);

	void txTask();
	void rxTask();

private:

	uint16_t maxMtu;
	MacLayer *ptrMacLayer;

	QueueHandle_t rxQueue;
};
