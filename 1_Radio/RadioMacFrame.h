#pragma once

#include <stdint.h>
#include "PoolNode.h"

class RadioMacFrame	: public PoolNode
{
public:
	RadioMacFrame()	 : PoolNode(2)
	{
	}
};
