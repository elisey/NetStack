#pragma once

#include <stdint.h>
#include "ncmp_layer_base.h"
#include "NcmpFrame.h"

class NcmpLayerMaster : public NcmpLayerBase
{
public:
	NcmpLayerMaster(NpLayer *_ptrNpLayer, interfaceType_t _interfaceType);
	void task();

	void sendPing(uint16_t dstAddress);

private:
	bool waitForPingAnswer(uint16_t targetAddress, unsigned int timeout);
	void waitForAnyPackets(uint32_t prevTick, unsigned int timeout);
	void deleteSlave(uint16_t slaveAddress);
	void parsePacket(NcmpFrame *packet, uint16_t srcAddress);
	void sendImMaster(uint16_t dstAddress);
	void parseAddRoutesPacket(NcmpFrame *packet);
	void parseDeleteRoutesPacket(NcmpFrame *packet);
	void parseMyRoutePacket(NcmpFrame *packet);
	void parsePongWithRoutesPacket(NcmpFrame *packet);
};
