#include "Routing.h"
#include "routeTable.h"
#include "NetStackBuilder.h"
#include "NetStackConfig.h"

Routing routing;

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

	if (dstAddress == np_BROADCAST_ADDRESS)	{
		int i;
		for (i = 0; i < NUM_OF_INTERFACES; ++i) {
			if (srcInterfaceId != i)	{
				if (interfaces[i] != NULL)	{
					NpFrame newFrame;
					newFrame.alloc();
					newFrame.copy(*ptrNpFrame);
					interfaces[i]->forward(&newFrame);
				}
			}
		}
	}
	else if (dstAddress == np_TOP_REDIRECTION_ADDRESS)	{
		if (srcInterfaceId != 0)	{
			if (interfaces[0] != NULL)	{
				NpFrame newFrame;
				newFrame.alloc();
				newFrame.copy(*ptrNpFrame);
				interfaces[0]->forward(&newFrame);
			}
		}
	}
	else	{
		uint8_t targetInterfaceId = routerTable.getInterfaceForDestinationAddress(dstAddress);
		if (targetInterfaceId != srcInterfaceId)	{
			if (interfaces[targetInterfaceId] != NULL)	{
				NpFrame newFrame;			//TODO по возможности избавиться от копрования
				newFrame.alloc();
				newFrame.copy(*ptrNpFrame);
				interfaces[targetInterfaceId]->forward(&newFrame);
			}
		}
	}
}

bool Routing::send(NpFrame *ptrNpFrame, uint16_t dstAddress, uint8_t ttl, NpFrame_ProtocolType_t protocolType)
{
	bool result;
	uint8_t targetInterfaceId = routerTable.getInterfaceForDestinationAddress(dstAddress);
	if (interfaces[targetInterfaceId] != NULL)	{
		result = interfaces[targetInterfaceId]->send(ptrNpFrame, dstAddress, ttl, protocolType);
	}
	else {
		ptrNpFrame->free();
		result = false;
	}
	return result;
}
