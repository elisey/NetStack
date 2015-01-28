#include "Routing.h"
#include "routeTable.h"
Routing::Routing()
{

}

void Routing::handleFrame(NpFrame* ptrNpFrame, uint8_t srcInterfaceId)
{
	uint8_t ttl = ptrNpFrame->getTtl();
	ttl--;
	if (ttl == 0)	{
		//drop
		return;
	}
	ptrNpFrame->setTtl(ttl);

	uint16_t dstAddress;
	dstAddress = ptrNpFrame->getDstAddress();
	if (dstAddress != BROADCAST_ADDRESS)	{
		uint8_t interfaceId = RouterTable::instance().getRouteForDstAddress(dstAddress);
		if (interfaceId == srcInterfaceId)	{
			//drop;
			return;
		}
		interfaces[interfaceId].forward(ptrNpFrame);
	}
	else	{
		int i;
		for (i = 0; i < NUM_OF_INTERFACES; ++i) {
			if (srcInterfaceId != i)	{
				interfaces[i].forward(ptrNpFrame);
			}
		}
	}


}


