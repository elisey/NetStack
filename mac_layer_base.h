#pragma once

#include "PoolNode.h"

class MacLayerBase {
public:
	virtual bool send( PoolNode *ptrPoolNode, uint16_t dstAddress) = 0;
	virtual bool receive( PoolNode *ptrPoolNode, unsigned int timeout) = 0;

	void setMtuSize(uint16_t newValue)
	{
		mtu = newValue;
	}
	uint16_t getMtuSize()
	{
		return mtu;
	}

private:
	uint16_t mtu;
};
