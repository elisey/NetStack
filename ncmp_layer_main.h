#pragma once

#include <stdint.h>
#include "ncmp_layer_base.h"
#include "NpFrame.h"

class NcmpLayerMain :public NcmpLayerBase
{
public:
	NcmpLayerMain(uint8_t _interfaceId, NpLayer *_ptrNpLayer);
	void task();
private:

	uint16_t waitForMaster();
	bool waitForPingAndReply();

	void sendPing(uint16_t dstAddress);
	void sendPong(uint16_t dstAddress);
	void sendPingWithRoutes(uint16_t dstAddress);
	void sendPongWithRoutes(uint16_t dstAddress);
	void sendImHere();

	uint32_t getTimeDelta(uint32_t prevTime, uint32_t currentTime);

	uint16_t currentMaster;
};
