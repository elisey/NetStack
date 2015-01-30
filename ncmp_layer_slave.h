#pragma once

#include <stdint.h>
#include "ncmp_layer_base.h"

class NcmpLayerSlave : public NcmpLayerBase
{
public:
	NcmpLayerSlave(uint8_t _interfaceId, NpLayer *_ptrNpLayer);
	void task();

private:
	bool waitForPingAnswer(uint16_t targetAddress);
	void parsePacket(NcmpFrame *packet, uint16_t srcAddress);
};
