#include "ncmp_layer_slave.h"
#include "NpFrame.h"
#include "NcmpFrame.h"

#include "FreeRTOS.h"
#include "task.h"

#include "routeTable.h"
#include "Routing.h"	//For MAX_TTL

#include "debug.h"

NcmpLayerSlave::NcmpLayerSlave(NpLayer *_ptrNpLayer, interfaceType_t _interfaceType)
	:	NcmpLayerBase( _ptrNpLayer, _interfaceType),
	 	currentMaster(0),
	 	pongCounter(0)
{
}

void NcmpLayerSlave::task()
{
	while(1)
	{
		if (currentMaster == 0)	{
			sendImSlave();
			uint16_t foundMaster = waitForMaster(IM_SLAVE_PACKET_PERIOD);
			if (foundMaster != 0)	{
				currentMaster = foundMaster;
				sendMyRoute(currentMaster);
				sendRoutes();
				RouterTable::instance().setDefaultGate(foundMaster);
			}
		}
		else	{
			if (waitForPingAndReply(WAIT_FOR_PING_PERIOD) == false)	{
				currentMaster = 0;
				RouterTable::instance().setDefaultGate(0);
			}
		}
	}
}

uint16_t NcmpLayerSlave::waitForMaster(unsigned int timeout)
{
	uint32_t prevTick = xTaskGetTickCount();
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, timeout - timeDelta);
		if (result == pdFAIL)	{
			return 0;
		}
		uint16_t srcAddress = npFrame.getSrcAddress();

		NcmpFrame ncmpFrame;
		ncmpFrame.clone(npFrame);

		if (ncmpFrame.getPacketType() == ncmpPacket_ImMaster)	{
			ncmpFrame.free();
			return srcAddress;
		}
		else	{
			//парсинг остальных пакетов (остальный пакетов нет)
			ncmpFrame.free();
		}
	} while(timeDelta < timeout);
	return 0;
}

bool NcmpLayerSlave::waitForPingAndReply(unsigned int timeout)
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

		if (srcAddress == currentMaster)	{
			if (ncmpFrame.getPacketType() == ncmpPacket_ping)	{
				pingReplay(srcAddress);
				ncmpFrame.free();
				return true;
			}
		}
		else	{
			//логирование
		}
		ncmpFrame.free();
	} while(timeDelta < timeout);
	return false;
}

void NcmpLayerSlave::sendMyRoute(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.createMyRoutePacket();

	ncmpFrame.insertEntryToRoutesPacket( selfAddress );

	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayerSlave::sendRoutes()
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.createAddRoutesPacket();

	ncmpFrame.insertEntryToRoutesPacket( selfAddress );
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
		uint16_t address = RouterTable::instance()[i].address;
		if (address != 0)	{
			ncmpFrame.insertEntryToRoutesPacket( RouterTable::instance()[i].address );
		}
	}
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, TOP_REDIRECTION_ADDRESS, MAX_TTL, NpFrame_NCMP);
}

void NcmpLayerSlave::pingReplay(uint16_t dstAddress)
{
	if (interfaceType == interfaceType_PointToPoint)	{
		pongCounter++;
		if (pongCounter >= 50)	{
			pongCounter = 0;
			sendPongWithRoutes(dstAddress);
		}
		else {
			sendPong(dstAddress);
		}
	}
	else if(interfaceType == interfaceType_Star)	{
		sendPong(dstAddress);
	}
}

void NcmpLayerSlave::sendPong(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_pong);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayerSlave::sendPongWithRoutes(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.createPongWithRoutesPacket();

	ncmpFrame.insertEntryToRoutesPacket( selfAddress );
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
		uint16_t address = RouterTable::instance()[i].address;
		if (address != 0)	{
			ncmpFrame.insertEntryToRoutesPacket( RouterTable::instance()[i].address );
		}
	}
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayerSlave::sendImSlave()
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ImSlave);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, BROADCAST_ADDRESS, 1, NpFrame_NCMP);
}

uint32_t NcmpLayerSlave::getTimeDelta(uint32_t prevTime, uint32_t currentTime)
{
	if (currentTime >= prevTime)	{
		return (currentTime - prevTime);
	}
	else	{
		return ( 0xffffffff - prevTime) + currentTime;
	}
}
