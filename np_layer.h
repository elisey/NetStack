#pragma once

#include <stdint.h>
#include "NpFrame.h"
#include "mac_layer_base.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Mutex.h"
#include "PacketAssembly.h"

extern uint16_t selfAddress;

#define USE_OWN_PACKET_ASSEMBLY		(1)
#define ROUTE_OTHER_PACKETS			(1)


class NpLayer
{
public:
	NpLayer(MacLayerBase *_ptrMacLayer, uint8_t _inderfaceId);

	void rxTask();
	bool send(NpFrame *ptrNpFrame, uint16_t dstAddess, uint8_t ttl, NpFrame_ProtocolType_t protocolType);
	bool forward(NpFrame *ptrNpFrame);
	uint8_t getInterfaceId();
	void setRxNcmpQueue(QueueHandle_t _rxNcmpQueue);

private:

	bool needRoutePacket(NpFrame *ptrNpFrame);
	bool needHandleOwnPacket(NpFrame *ptrNpFrame);
	void handleOwnFrame(NpFrame *ptrNpFrame);
	bool frameNeedAssemble(NpFrame *ptrNpFrame);
	bool processAssembling(NpFrame *ptrNpFrame);
	void parseOwnPacketByProtocol(NpFrame *ptrNpFrame);

	bool transfer(NpFrame *ptrNpFrame);
	uint16_t getNextHopAddress(NpFrame *ptrNpFrame);
	bool deassemblePacketAndSendParts(NpFrame *ptrNpFrame, uint16_t dstAddress, uint8_t payloadSize);
	void setAssemblyData(NpFrame *ptrNpFrame, uint8_t totalNumOfParts, uint8_t setCurrentPartIndex, uint8_t uniqueAssebleId);
	uint8_t calculateNumOfParts(uint8_t payloadSize);

	uint8_t getUniqueAssembleId();
	void processTp(NpFrame *npFrame);

	bool putFrameToQueue(NpFrame * ptrNpFrame, QueueHandle_t queue);

	MacLayerBase *ptrMacLayer;
	uint16_t maxNpPayload;
	uint8_t inderfaceId;

	QueueHandle_t rxNcmpQueue;
	QueueHandle_t rxTpQueue;
	QueueHandle_t rxTpaQueue;

	Mutex txMutex;

	PacketAssembly packetAssembly;
};
