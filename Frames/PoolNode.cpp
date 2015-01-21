#include "PoolNode.h"
#include "FramePool.h"
#include "debug.h"
PoolNode::PoolNode(size_t _bufferOffset)
	: memoryIndex(-1), bufferOffset(_bufferOffset)
{
}

void PoolNode::alloc()
{
	FramePool &pool = FramePool::instance();

	memoryIndex = pool.takeFreeFrameIndex();

	uint8_t *ptrData = pool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
}

bool PoolNode::allocFromIsr()
{
	FramePool &pool = FramePool::instance();

	int tempMemoryIndex = pool.takeFreeFrameIndexFromISR();
	if (tempMemoryIndex == (-1))	{
		//pin2_on;
		return false;
	}
	memoryIndex = tempMemoryIndex;
	uint8_t *ptrData = pool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
	return true;
}

void PoolNode::free()
{
	FramePool &pool = FramePool::instance();

	pool.releaseFrame(memoryIndex);
	buffer.setDataPtr(NULL);
}

void PoolNode::clone(PoolNode &node)
{
	FramePool &pool = FramePool::instance();

	size_t newLength = node.buffer.getLenght() + node.bufferOffset - bufferOffset;
	buffer.setLenght(newLength);

	memoryIndex = node.memoryIndex;
	node.memoryIndex = -1;
	node.buffer.setDataPtr(NULL);

	uint8_t *ptrData = pool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
}


