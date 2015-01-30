#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class NcmpLayerBase {
public:
	NcmpLayerBase(uint8_t _interfaceId, NpLayer *_ptrNpLayer);

	virtual void task() = 0;

protected:
	uint8_t interfaceId;
	QueueHandle_t rxQueue;
	NpLayer *ptrNpLayer;

private:

};
