#include "ncmp_layer_slave.h"
#include "NpFrame.h"
#include "NcmpFrame.h"

#include "FreeRTOS.h"
#include "task.h"

#include "routeTable.h"
#include <stdint.h>

NcmpLayerSlave::NcmpLayerSlave(uint8_t _interfaceId , NpLayer *_ptrNpLayer, interfaceType_t _interfaceType)
	:	NcmpLayerBase(_interfaceId, _ptrNpLayer, _interfaceType),
	 	currentMaster(0)
{
}

void NcmpLayerSlave::task()
{
	while(1)
	{
		if (currentMaster == 0)	{
			sendImSlave();
			uint16_t foundMaster = waitForMaster();
			if (foundMaster != 0)	{
				currentMaster = foundMaster;
				sendRoutes();
			}
		}
		else	{
			while(waitForPingAndReply() == false)	{
				currentMaster = 0;
			}
		}
	}
}

uint16_t NcmpLayerSlave::waitForMaster()
{
	uint32_t prevTick = xTaskGetTickCount();
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, 10 - timeDelta);
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
	} while(timeDelta < 10);
	return 0;
}

bool NcmpLayerSlave::waitForPingAndReply()
{
	uint32_t prevTick = xTaskGetTickCount();
	NpFrame npFrame;
	uint32_t timeDelta;
	do
	{
		timeDelta = getTimeDelta( prevTick, xTaskGetTickCount() );
		BaseType_t result = xQueueReceive(rxQueue, &npFrame, 50 - timeDelta);
		if (result == pdFAIL)	{
			return 0;
		}
		uint16_t srcAddress = npFrame.getSrcAddress();

		NcmpFrame ncmpFrame;
		ncmpFrame.clone(npFrame);

		if (srcAddress == currentMaster)	{
			if (ncmpFrame.getPacketType() == ncmpPacket_ping)	{
				sendPong(srcAddress);
				ncmpFrame.free();
				return true;
			}
			else if (ncmpFrame.getPacketType() == ncmpPacket_pingWithRoutes)	{
				sendPongWithRoutes(srcAddress);
				ncmpFrame.free();
				return true;
			}
		}
		ncmpFrame.free();
	} while(timeDelta < 50);
	return false;
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
