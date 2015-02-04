#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MacFrame.h"
#include "channel.h"
#include "UniqueFrame.h"

#define mac_layerRESEND_NUM		(3u)
#define mac_layerCRC_SIZE		(2u)

class MacLayer {
public:
	MacLayer(Channel *_ptrChannel);
	void rxTask();
	bool send(MacFrame &macFrame, packetAckType_t packetAckType);
	bool receive(MacFrame &macFrame, unsigned int timeout);

private:
	bool transfer(MacFrame &macFrame);
	void sendAck(uint8_t pid);
	void ackReceived(uint8_t pid);
	bool isAckReceived(uint8_t pid);
	void clearQueueAck();
	void handleRxPacket(MacFrame *ptrMacFrame);
	uint8_t getUniquePid();

	Channel *ptrChannel;
	UniqueFrame<uint8_t> uniqueFrame;
	QueueHandle_t rxQueue;
	QueueHandle_t ackQueue;
	SemaphoreHandle_t txMutex;
	uint8_t nextPid;
};
