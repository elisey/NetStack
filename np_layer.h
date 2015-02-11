#pragma once

#include <stdint.h>
#include "NpFrame.h"
#include "mac_layer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//#define selfAddress 0x33
extern uint16_t selfAddress;
#define MAX_TTL		32
class NpLayer
{
public:
	NpLayer(MacLayer *_ptrMacLayer, uint16_t _maxMtu, uint8_t _inderfaceId);

	void txTask();
	void rxTask();

	void setRxNcmpQueue(QueueHandle_t _rxNcmpQueue)
	{
		rxNcmpQueue = _rxNcmpQueue;
	}
	void setRxTpQueue(QueueHandle_t _rxTpQueue)
	{
		rxTpQueue = _rxTpQueue;
	}
	void setRxTpaQueue(QueueHandle_t _rxTpaQueue)
	{
		rxTpaQueue = _rxTpaQueue;
	}

	void send(NpFrame *ptrNpFrame,
			uint16_t dstAddess,
			uint8_t ttl,
			NpFrame_ProtocolType_t protocolType);
	void forward(NpFrame *ptrNpFrame);

private:

	void processTp(NpFrame *npFrame);

	bool putFrameToQueue(NpFrame * ptrNpFrame, QueueHandle_t queue);

	MacLayer *ptrMacLayer;
	uint16_t maxMtu;
	uint8_t inderfaceId;

	QueueHandle_t rxNcmpQueue;
	QueueHandle_t rxTpQueue;
	QueueHandle_t rxTpaQueue;

	QueueHandle_t txQueue;
};
