#pragma once

#include <stdint.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "queue.h"

class FramePool
{
public:
	FramePool();
	int takeFreeFrameIndex(uint32_t timeout);
	int takeFreeFrameIndexFromISR();
	uint8_t* getMemoryPtrByMemoryIndex(int memoryIndex);
	void releaseFrame(int memoryIndex);

private:
	QueueHandle_t freeFramesQueue;
	FramePool(const FramePool& root);
	FramePool& operator=(const FramePool&);
};

extern FramePool framePool;
