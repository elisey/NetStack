#pragma once

#include "PoolNode.h"

typedef enum	{
	packetAckType_withAck = 1,
	packetAckType_noAck = 2,
	packetAckType_Ack = 3
} packetAckType_t;

#define SFBus_FRAME_HEAD_LENGTH		(4)

class MacFrame: public PoolNode
{
public:
	MacFrame();

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
