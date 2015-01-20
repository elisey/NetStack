#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "MacFrame.h"

class MacLayer {
public:
	MacLayer(Channel *_ptrChannel);

	void task();

	void send(MacFrame *ptrMacFrame);
	bool receive(MacFrame *ptrMacFrame, unsigned int timeout);

	void handleRxPacket(MacFrame *ptrMacFrame);

private:

	void sendAck(uint8_t pid);

	Channel *ptrChannel;

	QueueHandle_t txQueue;
	QueueHandle_t rxQueue;
};
