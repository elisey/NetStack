#pragma once

#include "Buffer.h"
#include <stdint.h>

class PoolNode
{
public:
	PoolNode(size_t _bufferOffset);

	void alloc();
	bool allocFromIsr();
	void free();
	void clone(PoolNode &node);

protected:
	Buffer buffer;

private:
	uint8_t* allocMemory();
	void releaseMemry();
	uint8_t* cloneMemoryFromNode(PoolNode *ptrNode);

	int memoryIndex;
	const size_t bufferOffset;
};
