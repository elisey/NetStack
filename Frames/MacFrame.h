#pragma once

#include "PoolNode.h"

typedef enum	{
	packetAckType_withAck = 0,
	packetAckType_noAck,
	packetAckType_Ack
} packetAckType_t;

class MacFrame: public PoolNode
{
public:
	MacFrame();

	Buffer& getBuffer()
	{
		return buffer;
	}

	uint8_t getPid();
	void setPid(uint8_t newPid);

	packetAckType_t getPacketAckType();
	void setPacketAckType(packetAckType_t packetAckType);

	uint16_t getCrc();
	void setCrc(uint16_t newCrc);
	uint16_t calculateCrc();
	void calculateAndSetCrc();
	bool checkCrc();
};
