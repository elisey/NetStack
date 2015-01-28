#pragma once

#include <stdint.h>
#include "PoolNode.h"

typedef enum	{
	ncmpPacket_ping = 0,
	ncmpPacket_pong,
	ncmpPacket_im_here,
	ncmpPacket_giveRt,
	ncmpPacket_Rt
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
