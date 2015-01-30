#include "ncmp_layer.h"
#include "NpFrame.h"
#include "NcmpFrame.h"

#include <stdint.h>

NcmpLayer::NcmpLayer(uint8_t _interfaceId , NpLayer *_ptrNpLayer)
	:	NcmpLayerBase(_interfaceId, _ptrNpLayer),
	 	currentMaster(0)
{
}

void NcmpLayer::task()
{
	while(1)
	{
		if (currentMaster == 0)	{
			sendImHere();
			uint16_t foundMaster = waitForMaster();
			if (foundMaster != 0)	{
				currentMaster = foundMaster;
				sendRt(currentMaster);
			}
		}
		else	{
			int tryCounter = 0;
			while(waitForPingAndReply() == false)	{
				if(++tryCounter >= 3)	{
					currentMaster = 0;
					break;
				}
			}
		}
	}
}

uint16_t NcmpLayer::waitForMaster()
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
			return srcAddress;
		}
		else	{
			//парсинг остальных пакетов (остальный пакетов нет)
		}
	} while(timeDelta < 10);
	return 0;
}

bool NcmpLayer::waitForPingAndReply()
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

		if (srcAddress == currentMaster)	{
			if (ncmpFrame.getPacketType() == ncmpPacket_ping)	{
				sendPong(srcAddress);
				return true;
			}
			else if (ncmpFrame.getPacketType() == ncmpPacket_pingWithRoutes)	{
				sendPongWithRoutes(srcAddress);
				return true;
			}
		}
	} while(timeDelta < 10);
	return false;
}

void NcmpLayer::sendPong(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_pong);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayer::sendPongWithRoutes(uint16_t dstAddress)
{

}

void NcmpLayer::sendImHere()
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ImHere);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, BROADCAST_ADDRESS, 1, NpFrame_NCMP);
}

uint32_t NcmpLayer::getTimeDelta(uint32_t prevTime, uint32_t currentTime)
{
	if (currentTime >= prevTime)	{
		return (currentTime - prevTime);
	}
	else	{
		return ( 0xffffffff - prevTime) + currentTime;
	}
}
