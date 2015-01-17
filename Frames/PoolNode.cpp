#include "PoolNode.h"
#include "FramePool.h"

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
void PoolNode::free()
{
	FramePool &pool = FramePool::instance();

	pool.releaseFrame(memoryIndex);
	buffer.setDataPtr(NULL);
}

void PoolNode::clone(PoolNode &node)
{
	FramePool &pool = FramePool::instance();

	memoryIndex = node.memoryIndex;
	node.memoryIndex = 0;
	node.buffer.setDataPtr(NULL);

	uint8_t *ptrData = pool.getMemoryPtrByMemoryIndex(memoryIndex);
	buffer.setDataPtr(ptrData + bufferOffset);
}


