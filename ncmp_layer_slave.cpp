#include "ncmp_layer_slave.h"

#include "FreeRTOS.h"
#include "task.h"

#include "routeTable.h"

NcmpLayerSlave::NcmpLayerSlave(uint8_t _interfaceId, NpLayer* _ptrNpLayer)
	:	NcmpLayerBase(_interfaceId, _ptrNpLayer)
{
}

void NcmpLayerSlave::task()
{
	int prevRouteIndex = 0;
	while(1)
	{
		uint16_t targetAddress = 0;

		prevRouteIndex = RouterTable::instance().getNextRouteToPing(prevRouteIndex, interfaceId);
		if (prevRouteIndex != (-1))	{
			targetAddress = RouterTable::instance()[prevRouteIndex];
			send
		}

	}
}

bool NcmpLayerSlave::waitForPingAnswer(uint16_t targetAddress)
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

void NcmpLayerSlave::parsePacket(NcmpFrame *packet, uint16_t srcAddress)
{
	switch (packet->getPacketType())
	{
	case ncmpPacket_ImHere:
		SendImMaster(srcAddress);
		break;
	case ncmpPacket_AddRoutes:

		break;
	case ncmpPacket_DeleteRoutes:

		break;
	case ncmpPacket_pong:

		break;
	case ncmpPacket_pongWithRoutes:

		break;
	default:

		break;
	}
}

void NcmpLayerMain::sendImMaster(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ImMaster);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}
