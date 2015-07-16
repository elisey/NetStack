#pragma once
#include "Frame.h"
#include "channel.h"
#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "NetStackConfig.h"

class ChannelUartIrq : public Channel
{
//Общее
public:
	ChannelUartIrq();

	void transfer(Frame *ptrFrame);
	void waitForTransferCompleate();
	void startRx();
	void stopRx();

	void irqOnTxEmpty();
	void irqOnRxNotEmpty();
	void irqOnIdleReceived();

private:

	void uartInit();
	void gpioInit();
	void clearTxSemaphore();
	void txeIrqEnable();
	void txeIrqDisable();
	void transmiterEnable();
	void transmiterDisable();

	SemaphoreHandle_t  txSemaphore;
	uint8_t *ptrTxBuffer;
	unsigned int txByteCounter;


	Frame rxFrame;
	uint8_t *ptrRxBuffer;
	unsigned int rxByteCounter;

	void receiverEnable();
	void receiverDisable();

};

#if (USE_UART_5 == 1)
extern ChannelUartIrq channelUartIrq;
#endif
