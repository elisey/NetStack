#pragma once

#include <stdint.h>
#include "PoolNode.h"

#define TP_FRAME_HEAD_LENGTH		(4)

#define TP_FRAME_SRC_PORT			(0)
#define TP_FRAME_DST_PORT			(1)
#define TP_FRAME_UNIQUE_ID			(2)
#define TP_FRAME_FLAGS				(3)

#define TP_FRAME_FLAG_START			(0)
#define TP_FRAME_FLAG_STOP			(1)
#define TP_FRAME_FLAG_ACK			(2)
#define TP_FRAME_FLAG_REJECT		(3)

#define setBit(reg, bit)			((reg) |= (bit))
#define clearBit(reg, bit)			((reg) &= ~(bit))
#define readBit(reg, bit)			((reg) & (bit))

class TpFrame	: public PoolNode
{
public:
	TpFrame()	 : PoolNode(9)
	{
	}

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

	void setFlagStart(bool flag)
	{
		if (flag)	{
			setBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_START);
		}
		else	{
			clearBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_START);
		}
	}

	bool getFlagStart()
	{
		return ( readBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_START) != 0 );
	}

	void setFlagStop(bool flag)
	{
		if (flag)	{
			setBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_STOP);
		}
		else	{
			clearBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_STOP);
		}
	}

	bool getFlagStop()
	{
		return ( readBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_STOP) != 0 );
	}

	void setFlagAck(bool flag)
	{
		if (flag)	{
			setBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_ACK);
		}
		else	{
			clearBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_ACK);
		}
	}

	bool getFlagAck()
	{
		return ( readBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_ACK) != 0 );
	}

	void setFlagReject(bool flag)
	{
		if (flag)	{
			setBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_REJECT);
		}
		else	{
			clearBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_REJECT);
	}

	bool getFlagReject()
	{
		return ( readBit(buffer[TP_FRAME_FLAGS], 1 << TP_FRAME_FLAG_REJECT) != 0 );
	}

	uint8_t *getPayloadPtr()
	{
		return &(buffer[TP_FRAME_HEAD_LENGTH]);
	}

	unsigned int getPayloadSize()
	{
		return ( buffer.getLenght() - TP_FRAME_HEAD_LENGTH );
	}
};
