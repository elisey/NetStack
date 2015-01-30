#pragma once

/*
 * Net protocol Frame (NP Frame)
 */

#include <stdint.h>
#include "PoolNode.h"

typedef enum	{
	NpFrame_NCMP = 0,	// net control message protocol
	NpFrame_TPA = 1,	// transport protocol with acknowledgment
	NpFrame_TP = 2,		// transport protocol (without acknowledgment)
} NpFrame_ProtocolType_t;

#define BROADCAST_ADDRESS			(0)
#define TOP_REDIRECTION_ADDRESS		(0xFFFF)

class NpFrame : public PoolNode
{
public:
	NpFrame() : PoolNode(2)
	{
	}

	uint16_t getSrcAddress();
	void setSrcAddress(uint16_t newAddress);

	uint16_t getDstAddress();
	void setDstAddress(uint16_t newAddress);

	uint8_t getTtl();
	void setTtl(uint8_t newTtl);

	NpFrame_ProtocolType_t getProtocolType();
	void setProtocolType(NpFrame_ProtocolType_t protocolType);

};
