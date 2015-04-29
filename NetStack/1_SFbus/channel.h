#pragma once
#include "Frame.h"
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class Channel
{
public:
	Channel();
	void send(Frame *ptrFrame);
	bool receive(Frame *ptrFrame, unsigned int timeout);

	virtual void startRx() = 0;
	virtual void stopRx() = 0;

protected:
	virtual void transfer(Frame *ptrFrame) = 0;
	virtual void waitForTransferCompleate() = 0;
	void handleRxPacket(Frame *ptrFrame);

	Frame rxFrame;

private:
	QueueHandle_t rxQueue;
};
