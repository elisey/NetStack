#pragma once

#include <stdint.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "queue.h"

class FramePool
{
public:
	int takeFreeFrameIndex(uint32_t timeout);
	int takeFreeFrameIndexFromISR();
	uint8_t* getMemoryPtrByMemoryIndex(int memoryIndex);
	void releaseFrame(int memoryIndex);

private:
	QueueHandle_t freeFramesQueue;

public:
	static FramePool& instance()
	{
		static FramePool theSingleInstance;
		return theSingleInstance;
	}
private:
	FramePool();
	FramePool(const FramePool& root);
	FramePool& operator=(const FramePool&);
};
