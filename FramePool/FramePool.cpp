#include "FramePool.h"
#include "NetStackConfig.h"

FramePool framePool;

uint8_t framesPool[netConfigMAX_FRAME_SIZE * netConfigNUM_OF_FRAMES];

FramePool::FramePool()
{
	freeFramesQueue = xQueueCreate(netConfigNUM_OF_FRAMES, sizeof(int));

	int i;
	for (i = 0; i < netConfigNUM_OF_FRAMES; ++i) {
		BaseType_t result = xQueueSend(freeFramesQueue, &i, 0);
		assert(result == pdPASS);
	}
}

int FramePool::takeFreeFrameIndex(uint32_t timeout)
{
	BaseType_t result;
	int freeFrameIndex = -1;
	result = xQueueReceive(freeFramesQueue, &freeFrameIndex, ( TickType_t ) timeout);
	if (result != pdPASS)	{
		return (-1);
	}
	return freeFrameIndex;
}

int FramePool::takeFreeFrameIndexFromISR()
{
	BaseType_t result;
	int freeFrameIndex = -1;
	result = xQueueReceiveFromISR(freeFramesQueue, &freeFrameIndex, NULL);
	if (result != pdPASS)	{
		return (-1);
	}
	return freeFrameIndex;
}

uint8_t* FramePool::getMemoryPtrByMemoryIndex(int memoryIndex)
{
	if (memoryIndex < 0)	{
		return NULL;
	}

	assert(memoryIndex < netConfigNUM_OF_FRAMES);
	return &(framesPool[memoryIndex * netConfigMAX_FRAME_SIZE]);
}

void FramePool::releaseFrame(int memoryIndex)
{
	if (memoryIndex < 0)	{
		return;
	}
	BaseType_t result;
	result = xQueueSend(freeFramesQueue, &memoryIndex, ( TickType_t ) 0);
	assert(result == pdPASS);
}
