#pragma once

#include <stdint.h>
#include "PoolNode.h"
#include "NetConfig.h"

#define TP_FRAME_SRC_PORT			(0)
#define TP_FRAME_DST_PORT			(1)
#define TP_FRAME_UNIQUE_ID			(2)
#define TP_FRAME_TYPE				(3)

typedef enum	{
	TpFrameType_Connect = 1,
	TpFrameType_Disconnect,
	TpFrameType_Ack,
	TpFrameType_Reject,
	TpFrameType_Data
}	TpFrameType_t;

class TpFrame	: public PoolNode
{
public:
	TpFrame()	 : PoolNode(9)
	{
	}

	uint16_t srcAddress;

	void setSrcPort(uint8_t port)
	{
		buffer[TP_FRAME_SRC_PORT] = port;
	}

	uint8_t getSrcPort()
	{
		return (buffer[TP_FRAME_SRC_PORT]);
	}

	void setDstPort(uint8_t port)
	{
		buffer[TP_FRAME_DST_PORT] = port;
	}

	uint8_t getDstPort()
	{
		return (buffer[TP_FRAME_DST_PORT]);
	}

	void setUniqueId(uint8_t uniqueId)
	{
		buffer[TP_FRAME_UNIQUE_ID] = uniqueId;
	}

	uint8_t getUniqueId()
	{
		return (buffer[TP_FRAME_UNIQUE_ID]);
	}

	TpFrameType_t getType()
	{
		TpFrameType_t result;
		result = static_cast< TpFrameType_t >(buffer[TP_FRAME_TYPE]);
		return result;
	}

	void setType (TpFrameType_t newType)
	{
		buffer[TP_FRAME_TYPE] = static_cast< uint8_t >(newType);
	}

	uint8_t *getPayloadPtr()
	{
		return &(buffer[tp_FRAME_HEAD_LENGTH]);
	}

	unsigned int getPayloadSize()
	{
		return ( buffer.getLenght() - tp_FRAME_HEAD_LENGTH );
	}
};
