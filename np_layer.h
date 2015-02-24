#pragma once

#include <stdint.h>
#include "NpFrame.h"
#include "mac_layer_base.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "PacketAssembly.h"

extern uint16_t selfAddress;
#define MAX_TTL		32

class NpLayer
{
public:
	NpLayer(MacLayerBase *_ptrMacLayer, uint8_t _inderfaceId);

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

	uint8_t getInterfaceId()
	{
		return inderfaceId;
	}

private:
	uint8_t getUniqueAssembleId();
	void processTp(NpFrame *npFrame);

	bool putFrameToQueue(NpFrame * ptrNpFrame, QueueHandle_t queue);

	MacLayerBase *ptrMacLayer;
	uint16_t maxNpMtu;
	uint8_t inderfaceId;

	QueueHandle_t rxNcmpQueue;
	QueueHandle_t rxTpQueue;
	QueueHandle_t rxTpaQueue;

	QueueHandle_t txQueue;

	PacketAssembly packetAssembly;
};
