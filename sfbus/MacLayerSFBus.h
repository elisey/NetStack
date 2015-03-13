#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "MacFrame.h"
#include "channel.h"
#include "UniqueItemHandler.h"

#include "mac_layer_base.h"
#include "Mutex.h"

class MacLayerSFBus :	public MacLayerBase
{
public:
	MacLayerSFBus(Channel *_ptrChannel);
	void rxTask();
	bool send(PoolNode *ptrPoolNode, uint16_t dstAddress);
	bool receive(PoolNode *ptrPoolNode, unsigned int timeout);

private:
	bool transfer(MacFrame &macFrame);
	void sendAck(uint8_t pid);
	void ackReceived(uint8_t pid);
	bool isAckReceived(uint8_t pid, unsigned int timeout);
	void clearQueueAck();
	void handleRxPacket(MacFrame *ptrMacFrame);
	uint8_t getUniquePid();

	Channel *ptrChannel;
	UniqueItemHandler<uint8_t> uniqueFrame;
	QueueHandle_t rxQueue;
	QueueHandle_t ackQueue;
	Mutex txMutex;
	uint8_t nextPid;
};
