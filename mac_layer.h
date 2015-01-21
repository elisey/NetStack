#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "MacFrame.h"
#include "channel.h"

class MacLayer {
public:
	MacLayer(Channel *_ptrChannel);

	void txTask();
	void rxTask();

	void send(MacFrame *ptrMacFrame);
	bool receive(MacFrame *ptrMacFrame, unsigned int timeout);



private:

	void sendAck(uint8_t pid);
	void ackReceived(uint8_t pid);
	bool isAckReceived(uint8_t pid);
	void handleRxPacket(MacFrame *ptrMacFrame);

	Channel *ptrChannel;

	QueueHandle_t txQueue;
	QueueHandle_t rxQueue;
	QueueHandle_t ackQueue;
};
