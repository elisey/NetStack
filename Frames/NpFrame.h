#pragma once

/*
 * Net protocol Frame (NP Frame)
 */

#include <stdint.h>
#include <string.h>
#include "PoolNode.h"
#include "debug.h"
typedef enum	{
	NpFrame_NCMP = 1,	// net control message protocol
	NpFrame_TPA = 2,	// transport protocol with acknowledgment
	NpFrame_TP = 3,		// transport protocol (without acknowledgment)
} NpFrame_ProtocolType_t;

#define BROADCAST_ADDRESS			(0)
#define TOP_REDIRECTION_ADDRESS		(0xFFFF)
#define NP_FRAME_HEAD_LENGTH		(7)

class NpFrame : public PoolNode
{
public:
	NpFrame() : PoolNode(2)
	{
	}

	uint16_t getSrcAddress()
	{
		uint16_t address;

		address = (uint16_t)buffer[0] << 8;
		address |= (uint16_t)buffer[1];
		return address;
	}
	void setSrcAddress(uint16_t newAddress)
	{
		buffer[0] = (uint8_t)(newAddress >> 8);
		buffer[1] = (uint8_t)(newAddress & 0xff);
	}

	uint16_t getDstAddress()
	{
		uint16_t address;

		address = (uint16_t)buffer[2] << 8;
		address |= (uint16_t)buffer[3];
		return address;
	}
	void setDstAddress(uint16_t newAddress)
	{
		buffer[2] = (uint8_t)(newAddress >> 8);
		buffer[3] = (uint8_t)(newAddress & 0xff);
	}

#define TTL_MASK			(0b11111000)
#define PROTOCOL_TYPE_MASK	(0b00000111)

	uint8_t getTtl()
	{
		uint8_t ttl;
		ttl = ( ( buffer[4] >> 3 ) & 0b11111);
		return ttl;
	}
	void setTtl(uint8_t newTtl)
	{
		uint8_t temp;
		temp = buffer[4];
		temp &= ~TTL_MASK;
		temp |= (newTtl << 3);
		buffer[4] = temp;
	}

	NpFrame_ProtocolType_t getProtocolType()
	{
		NpFrame_ProtocolType_t result;

		result = static_cast<NpFrame_ProtocolType_t>( buffer[4] & PROTOCOL_TYPE_MASK );
		return result;
	}
	void setProtocolType(NpFrame_ProtocolType_t protocolType)
	{
		uint8_t temp;
		temp = buffer[4];
		temp &= ~PROTOCOL_TYPE_MASK;
		temp |= static_cast<uint8_t>(protocolType);
		buffer[4] = temp;
	}

#define TOTAL_NUM_OF_PARTS_MASK	(0b11110000)
#define CURRENT_PART_INDEX_MASK	(0b00001111)
	uint8_t getTotalNumOfParts()
	{
		uint8_t numOfParts;
		numOfParts = ( (buffer[5] >> 4) & (0b1111) );
		return numOfParts;
	}

	void setTotalNumOfParts(uint8_t numOfParts)
	{
		assert(numOfParts <= 0b1111);

		uint8_t reg;
		reg = buffer[5];
		reg &= ~TOTAL_NUM_OF_PARTS_MASK;

		reg |= (numOfParts << 4);
		buffer[5] = reg;
	}

	uint8_t getCurrentPartIndex()
	{
		uint8_t currentIndex;
		currentIndex = ( buffer[5] & (0b1111) );
		return currentIndex;
	}

	void setCurrentPartIndex(uint8_t currentIndex)
	{
		assert(currentIndex <= 0b1111);

		uint8_t reg;
		reg = buffer[5];
		reg &= ~CURRENT_PART_INDEX_MASK;

		reg |= (currentIndex);
		buffer[5] = reg;
	}

	uint8_t getUniqueAssembleId()
	{
		return buffer[6];
	}

	void setUniqueAssembleId(uint8_t id)
	{
		buffer[6] = id;
	}

	uint8_t *getPayloadPtr()
	{
		return &(buffer[NP_FRAME_HEAD_LENGTH]);
	}

	unsigned int getPayloadSize()
	{
		return ( buffer.getLenght() - NP_FRAME_HEAD_LENGTH );
	}

	void copyHead(NpFrame *ptrNpFrameSource)
	{
		uint8_t *src = ptrNpFrameSource->getBuffer().getDataPtr();
		uint8_t *dst = buffer.getDataPtr();

		memcpy( dst, src, NP_FRAME_HEAD_LENGTH );
	}
};
