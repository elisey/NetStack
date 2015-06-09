#pragma once

#include "Buffer.h"
#include <stdint.h>
#include "FreeRTOS.h"

class PoolNode
{
public:
	PoolNode(size_t _bufferOffset);

	bool alloc(uint32_t timeout = 50/*portMAX_DELAY*/);
	bool allocFromIsr();
	void free();
	void clone(PoolNode &node);
	void copy(PoolNode &node);
	Buffer& getBuffer();

protected:
	Buffer buffer;

private:
	uint8_t* allocMemory();
	void releaseMemry();
	uint8_t* cloneMemoryFromNode(PoolNode *ptrNode);

	int memoryIndex;
	size_t bufferOffset;
};
