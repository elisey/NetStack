#include "PoolNode.h"
#include "FramePool.h"
#include "debug.h"

#include <string.h>

static int frameCounter = 0;

PoolNode::PoolNode(size_t _bufferOffset)
	: memoryIndex(-1), bufferOffset(_bufferOffset)
{
}

bool PoolNode::alloc(uint32_t timeout)
{
	int tempMemoryIndex = framePool.takeFreeFrameIndex(timeout);
	if (tempMemoryIndex == -1)	{
		if (timeout != 0)	{
			assert(0);
		}
		return false;
	}
	memoryIndex = tempMemoryIndex;
	uint8_t *ptrData = framePool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
	frameCounter++;
	return true;
}

bool PoolNode::allocFromIsr()
{
	int tempMemoryIndex = framePool.takeFreeFrameIndexFromISR();
	if (tempMemoryIndex == (-1))	{
		//pin2_on;
		return false;
	}
	memoryIndex = tempMemoryIndex;
	uint8_t *ptrData = framePool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
	frameCounter++;
	return true;
}

void PoolNode::free()
{
	framePool.releaseFrame(memoryIndex);
	memoryIndex = -1;
	buffer.setDataPtr(NULL);
	frameCounter--;
}

void PoolNode::clone(PoolNode &node)
{
	size_t newLength = node.buffer.getLenght() + node.bufferOffset - bufferOffset;
	buffer.setLenght(newLength);

	memoryIndex = node.memoryIndex;
	node.memoryIndex = -1;
	node.buffer.setDataPtr(NULL);

	uint8_t *ptrData = framePool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
}

void PoolNode::copy(PoolNode &node)
{
	assert(bufferOffset == node.bufferOffset);
	memcpy( buffer.getDataPtr() - bufferOffset, node.buffer.getDataPtr() - node.bufferOffset, node.buffer.getLenght() + node.bufferOffset );
	buffer.setLenght( node.buffer.getLenght());
}

Buffer& PoolNode::getBuffer()
{
	return buffer;
}
