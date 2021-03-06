#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "np_layer.h"

typedef enum	{
	interfaceType_PointToPoint,
	interfaceType_Star
} interfaceType_t;

typedef enum	{
	interfaceConnectionState_Connected,
	interfaceConnectionState_Disconnected
} interfaceConnectionState_t;

class NcmpLayerBase {
public:
	NcmpLayerBase(NpLayer *_ptrNpLayer, interfaceType_t _interfaceType);
	interfaceConnectionState_t getInterfaceConnectionState();
	virtual void task() = 0;

protected:
	uint8_t interfaceId;
	QueueHandle_t rxQueue;
	NpLayer *ptrNpLayer;
	interfaceType_t interfaceType;
	interfaceConnectionState_t interfaceState;

private:

};
