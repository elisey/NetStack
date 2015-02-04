#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "np_layer.h"

typedef enum	{
	interfaceType_PointToPoint,
	interfaceType_Star
} interfaceType_t;

class NcmpLayerBase {
public:
	NcmpLayerBase(uint8_t _interfaceId, NpLayer *_ptrNpLayer, interfaceType_t _interfaceType);

	virtual void task() = 0;

protected:
	uint8_t interfaceId;
	QueueHandle_t rxQueue;
	NpLayer *ptrNpLayer;
	interfaceType_t interfaceType;

private:

};
