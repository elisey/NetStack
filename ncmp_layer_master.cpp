#include "ncmp_layer_master.h"

#include "FreeRTOS.h"
#include "task.h"

#include "routeTable.h"
#include "Routing.h"

uint32_t getTimeDelta(uint32_t prevTime, uint32_t currentTime);

NcmpLayerMaster::NcmpLayerMaster(uint8_t _interfaceId, NpLayer* _ptrNpLayer, interfaceType_t _interfaceType)
	:	NcmpLayerBase(_interfaceId, _ptrNpLayer, _interfaceType)
{
}

void NcmpLayerMaster::task()
{
	int prevRouteIndex = 0;
	while(1)
	{
		uint16_t targetAddress = 0;
		uint32_t prevTick = xTaskGetTickCount();

		prevRouteIndex = RouterTable::instance().getNextRouteToPing(prevRouteIndex, interfaceId);
		if (prevRouteIndex != (-1))	{
			targetAddress = (RouterTable::instance())[prevRouteIndex].address;
			sendPing(targetAddress);
			if (waitForPingAnswer(targetAddress) == false)	{

				uint16_t routeToDelete;
				NcmpFrame ncmpFrame;
				ncmpFrame.alloc();
				ncmpFrame.createDeleteRoutesPacket();

				if (interfaceType == interfaceType_Star)	{
					routeToDelete = targetAddress;

					RouterTable::instance().deleteRoute(routeToDelete);
					ncmpFrame.insertEntryToRoutesPacket(routeToDelete);

				}
				else	{
					routeToDelete = RouterTable::instance().findRouteForInterface(interfaceId);

					while(routeToDelete != (0))	{
						RouterTable::instance().deleteRoute(routeToDelete);
						ncmpFrame.insertEntryToRoutesPacket(routeToDelete);

						routeToDelete = RouterTable::instance().findRouteForInterface(interfaceId);
					}
				}

				NpFrame npFrame;
				npFrame.clone(ncmpFrame);
				npFrame.setDstAddress(TOP_REDIRECTION_ADDRESS);
				npFrame.setSrcAddress(selfAddress);
				npFrame.setProtocolType(NpFrame_NCMP);
				npFrame.setTtl(MAX_TTL);

				Routing::instance().handleFrame(&npFrame, interfaceId);
				npFrame.free();
			}
		}
		waitForAnyPackets(prevTick);
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

bool NcmpLayerMaster::waitForPingAnswer(uint16_t targetAddress)
{
	uint32_t prevTick = xTaskGetTickCount();
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, 10 - timeDelta);
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

	} while(timeDelta < 10);
	return false;
}

void NcmpLayerMaster::waitForAnyPackets(uint32_t prevTick)
{
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, 20 - timeDelta);
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
	} while(timeDelta < 20);
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
		//распарсить и перезаписать роуты
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
		RouterTable::instance().insertRoute(addressToAdd, interfaceId, false);
	}
}

void NcmpLayerMaster::parseDeleteRoutesPacket(NcmpFrame *packet)
{
	uint16_t addressToDelete = 0;

	int numOfEntries = packet->getNumOfEntriesInRouterPacket();
	for (int i = 0; i < numOfEntries; ++i) {
		addressToDelete = packet->getEntryFromRoutesPacketByIndex(i);
		RouterTable::instance().deleteRoute(addressToDelete);
	}
}

void NcmpLayerMaster::parseMyRoutePacket(NcmpFrame *packet)
{
	uint16_t addressToAdd = 0;
	addressToAdd = packet->getEntryFromRoutesPacketByIndex(0);
	RouterTable::instance().insertRoute(addressToAdd, interfaceId, true);
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
