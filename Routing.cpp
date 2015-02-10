#include "Routing.h"
#include "routeTable.h"
#include "config.h"
Routing::Routing()
{
}

void Routing::handleFrame(NpFrame* ptrNpFrame, uint8_t srcInterfaceId)
{
	uint8_t ttl = ptrNpFrame->getTtl();
	ttl--;
	if (ttl == 0)	{
		return;
	}
	ptrNpFrame->setTtl(ttl);

	uint16_t dstAddress;
	dstAddress = ptrNpFrame->getDstAddress();

	if (dstAddress == BROADCAST_ADDRESS)	{
		int i;
		for (i = 0; i < NUM_OF_INTERFACES; ++i) {
			if (srcInterfaceId != i)	{
				NpFrame newFrame;
				newFrame.alloc();
				newFrame.copy(*ptrNpFrame);
				interfaces[i]->forward(&newFrame);
			}
		}
	}
	else if (dstAddress == TOP_REDIRECTION_ADDRESS)	{
		if (srcInterfaceId != 0)	{
			NpFrame newFrame;
			newFrame.alloc();
			newFrame.copy(*ptrNpFrame);
			interfaces[0]->forward(&newFrame);
		}
	}
	else	{
		uint8_t targetInterfaceId = RouterTable::instance().getInterfaceForDestinationAddress(dstAddress);
		if (targetInterfaceId != srcInterfaceId)	{
			interfaces[targetInterfaceId]->forward(ptrNpFrame);
		}
	}
}