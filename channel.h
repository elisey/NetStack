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
	void send(const Frame *ptrFrame);
	bool receive(Frame *ptrFrame, unsigned int timeout);
	void txTask();

protected:
	virtual void transfer(Frame *ptrFrame) = 0;
	virtual bool isTxBusy() = 0;
	virtual void startRx() = 0;
	virtual void stopRx() = 0;

	void handleRxPacket(Frame *ptrFrame);

	Frame rxFrame;

private:
	QueueHandle_t txQueue;
	QueueHandle_t rxQueue;
};
