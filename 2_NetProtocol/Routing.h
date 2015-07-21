#pragma once
#include <stdint.h>
#include "NpFrame.h"

class Routing	{
public:
	Routing();
	void handleFrame(NpFrame *ptrNpFrame, uint8_t srcInterfaceId);
	bool send(NpFrame *ptrNpFrame, uint16_t dstAddess, uint8_t ttl, NpFrame_ProtocolType_t protocolType);

private:
	Routing(const Routing& root);
	Routing& operator=(const Routing&);
};

extern Routing routing;
