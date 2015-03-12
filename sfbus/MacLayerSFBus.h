#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MacFrame.h"
#include "channel.h"
#include "UniqueItemHandler.h"

#include "mac_layer_base.h"

#define mac_layerRESEND_NUM				(3u)
#define mac_layerCRC_SIZE				(2u)
#define mac_layerWAIT_ACK_TIMEOUT		(2u)
#define mac_layerMAX_PAYLOAD_SIZE		(MAX_SFBus_PACKET_SIZE - SFBus_FRAME_HEAD_LENGTH)
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
	SemaphoreHandle_t txMutex;
	uint8_t nextPid;
};
