#pragma once

#include "PoolNode.h"

class MacLayerBase {
public:
	virtual bool send( PoolNode *ptrPoolNode, uint16_t dstAddress) = 0;
	virtual bool receive( PoolNode *ptrPoolNode, unsigned int timeout) = 0;
};
