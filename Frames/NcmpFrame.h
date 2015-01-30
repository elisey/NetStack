#pragma once

#include <stdint.h>
#include "PoolNode.h"

typedef enum	{
	ncmpPacket_ImHere = 0,
	ncmpPacket_ImMaster,
	ncmpPacket_AddRoutes,
	ncmpPacket_DeleteRoutes,
	ncmpPacket_ping,
	ncmpPacket_pong,
	ncmpPacket_pingWithRoutes,
	ncmpPacket_pongWithRoutes,
}ncmpPacket_t;

class NcmpFrame	: public PoolNode
{
public:
	NcmpFrame()	 : PoolNode(9)
	{
	}

	ncmpPacket_t getPacketType()
	{
		return static_cast<ncmpPacket_t>(buffer[0]);
	}
	void setPacketType(ncmpPacket_t packetType)
	{
		buffer[0] = static_cast<uint8_t>(packetType);
	}

private:

};
