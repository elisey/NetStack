#include "ncmp_layer_master.h"

#include "routeTable.h"
#include "Routing.h"
#include "NetStackConfig.h"

uint32_t getTimeDelta(uint32_t prevTime, uint32_t currentTime);

NcmpLayerMaster::NcmpLayerMaster(NpLayer* _ptrNpLayer, interfaceType_t _interfaceType)
	:	NcmpLayerBase(_ptrNpLayer, _interfaceType)
{
}

void NcmpLayerMaster::task()
{
	int prevRouteIndex = 0;
	while(1)
	{
		uint16_t targetAddress = 0;
		uint32_t prevTick = xTaskGetTickCount();

		prevRouteIndex = routerTable.getNextRouteToPing(prevRouteIndex, interfaceId);
		if (prevRouteIndex != (-1))	{
			interfaceState = interfaceConnectionState_Connected;
			targetAddress = (routerTable)[prevRouteIndex].address;

			bool pingResult;
			int i;
			for (i = 0; i < ncmp_NUM_OF_PING_TRY; ++i) {
				sendPing(targetAddress);
				pingResult = waitForPingAnswer(targetAddress, ncmp_WAIT_FOR_PONG_TIMEOUT);
				if (pingResult == true)	{
					break;
				}
			}

			if (pingResult == false)	{
				deleteSlave(targetAddress);
			}
		}
		else	{
			interfaceState = interfaceConnectionState_Disconnected;
		}
		waitForAnyPackets(prevTick, ncmp_PING_PERIOD);
	}
}

void NcmpLayerMaster::sendPing(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ping);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

bool NcmpLayerMaster::waitForPingAnswer(uint16_t targetAddress, unsigned int timeout)
{
	uint32_t prevTick = xTaskGetTickCount();
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, timeout - timeDelta);
		if (result == pdFAIL)	{
			return false;
		}
		uint16_t srcAddress = npFrame.getSrcAddress();

		NcmpFrame ncmpFrame;
		ncmpFrame.clone(npFrame);

		if (targetAddress == srcAddress)	{
			if (ncmpFrame.getPacketType() == ncmpPacket_pong)	{
				parsePacket(&ncmpFrame, srcAddress);
				return true;
			}
			else if (ncmpFrame.getPacketType() == ncmpPacket_pongWithRoutes)	{
				parsePacket(&ncmpFrame, srcAddress);
				return true;
			}
		}
		parsePacket(&ncmpFrame, srcAddress);

	} while(timeDelta < timeout);
	return false;
}

void NcmpLayerMaster::waitForAnyPackets(uint32_t prevTick, unsigned int timeout)
{
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		if (timeout <= timeDelta)	{
			return;
		}
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, timeout - timeDelta);
		if (result == pdFAIL)	{
			return;
		}
		uint16_t srcAddress = npFrame.getSrcAddress();

		NcmpFrame ncmpFrame;
		ncmpFrame.clone(npFrame);

		if (ncmpFrame.getPacketType() == ncmpPacket_AddRoutes)	{
			parsePacket(&ncmpFrame, srcAddress);
			return;
		}
		parsePacket(&ncmpFrame, srcAddress);
	} while(timeDelta < timeout);
}

void NcmpLayerMaster::deleteSlave(uint16_t slaveAddress)
{
	uint16_t routeToDelete;
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.createDeleteRoutesPacket();

	if (interfaceType == interfaceType_Star)	{
		routeToDelete = slaveAddress;

		routerTable.deleteRoute(routeToDelete);
		ncmpFrame.insertEntryToRoutesPacket(routeToDelete);

	}
	else	{
		routeToDelete = routerTable.findRouteForInterface(interfaceId);

		while(routeToDelete != (0))	{
			routerTable.deleteRoute(routeToDelete);
			ncmpFrame.insertEntryToRoutesPacket(routeToDelete);

			routeToDelete = routerTable.findRouteForInterface(interfaceId);
		}
	}

	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	routing.send(&npFrame, np_TOP_REDIRECTION_ADDRESS, np_MAX_TTL, NpFrame_NCMP);
}

void NcmpLayerMaster::parsePacket(NcmpFrame *packet, uint16_t srcAddress)
{
	switch (packet->getPacketType())
	{
	case ncmpPacket_ImSlave:
		sendImMaster(srcAddress);
		break;
	case ncmpPacket_AddRoutes:
		parseAddRoutesPacket(packet);
		break;
	case ncmpPacket_DeleteRoutes:
		parseDeleteRoutesPacket(packet);
		break;
	case ncmpPacket_MyRoute:
		parseMyRoutePacket(packet);
		break;
	case ncmpPacket_pong:

		break;
	case ncmpPacket_pongWithRoutes:
		parsePongWithRoutesPacket(packet);
		break;
	default:

		break;
	}
	packet->free();
}

void NcmpLayerMaster::sendImMaster(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ImMaster);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayerMaster::parseAddRoutesPacket(NcmpFrame *packet)
{
	uint16_t addressToAdd = 0;
	int numOfEntries = packet->getNumOfEntriesInRouterPacket();

	for (int i = 0; i < numOfEntries; ++i) {
		addressToAdd = packet->getEntryFromRoutesPacketByIndex(i);
		routerTable.insertRoute(addressToAdd, interfaceId, false);
	}
}

void NcmpLayerMaster::parseDeleteRoutesPacket(NcmpFrame *packet)
{
	uint16_t addressToDelete = 0;

	int numOfEntries = packet->getNumOfEntriesInRouterPacket();
	for (int i = 0; i < numOfEntries; ++i) {
		addressToDelete = packet->getEntryFromRoutesPacketByIndex(i);
		routerTable.deleteRoute(addressToDelete);
	}
}

void NcmpLayerMaster::parseMyRoutePacket(NcmpFrame *packet)
{
	uint16_t addressToAdd = 0;
	addressToAdd = packet->getEntryFromRoutesPacketByIndex(0);
	routerTable.insertRoute(addressToAdd, interfaceId, true);
}

void NcmpLayerMaster::parsePongWithRoutesPacket(NcmpFrame *packet)
{
	int i;
	for (i = 0; i < rt_ROUTER_TABLE_SIZE; ++i) {
		if ( ( routerTable[i].interfaceId == interfaceId) &&
			( routerTable[i].isNeighbor == false  ))	{
			routerTable[i].clear();
		}
	}

	uint16_t addressToAdd = 0;
	int numOfEntries = packet->getNumOfEntriesInRouterPacket();

	for (int i = 0; i < numOfEntries; ++i) {
		addressToAdd = packet->getEntryFromRoutesPacketByIndex(i);
		routerTable.insertRoute(addressToAdd, interfaceId, false);
	}
}

uint32_t getTimeDelta(uint32_t prevTime, uint32_t currentTime)
{
	if (currentTime >= prevTime)	{
		return (currentTime - prevTime);
	}
	else	{
		return ( 0xffffffff - prevTime) + currentTime;
	}
}
