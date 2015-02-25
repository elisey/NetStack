#pragma once

#include <stdint.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "queue.h"

#include "channel_UART.h"

#define MAX_MTU				(MAX_RS_PACKET_SIZE)
#define NUM_OF_FRAMES		(60)

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
