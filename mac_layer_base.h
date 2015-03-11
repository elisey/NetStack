#pragma once

#include "PoolNode.h"

class MacLayerBase {
public:
	virtual bool send( PoolNode *ptrPoolNode, uint16_t dstAddress) = 0;
	virtual bool receive( PoolNode *ptrPoolNode, unsigned int timeout) = 0;

	void setMaxPayloadSize(uint16_t newValue)
	{
		maxPayloadSize = newValue;
	}
	uint16_t getMaxPayloadSize()
	{
		return maxPayloadSize;
	}

private:
	uint16_t maxPayloadSize;
};
