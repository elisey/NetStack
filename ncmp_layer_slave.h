#pragma once

#include <stdint.h>
#include "ncmp_layer_base.h"

class NcmpLayerSlave :public NcmpLayerBase
{
public:
	NcmpLayerSlave(uint8_t _interfaceId, NpLayer *_ptrNpLayer, interfaceType_t _interfaceType);
	void task();
private:

	uint16_t waitForMaster();
	bool waitForPingAndReply();

	void sendRoutes();
	void sendPong(uint16_t dstAddress);
	void sendPongWithRoutes(uint16_t dstAddress);
	void sendImSlave();

	uint32_t getTimeDelta(uint32_t prevTime, uint32_t currentTime);

	uint16_t currentMaster;
};
