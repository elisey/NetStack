#pragma once
#include "Frame.h"
#include "channel.h"
#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define MAX_PACKET_SIZE		(256)	//TODO единый размер пакета
#define MIN_PACKET_SIZE		(4)

typedef enum	{
	uart_channel_1,
	uart_channel_2,
	uart_channel_3,
	uart_channel_4
} uart_channel_t;

class ChannelUart : public Channel
{
//Общее
public:
	ChannelUart(uart_channel_t ch_uart);

	void transfer(Frame *ptrFrame);
	void waitForTransferCompleate();
	void startRx();
	void stopRx();
	void irqOnTxCompleate();

private:
	void dmaTxInit();
	void dmaTxStart(uint8_t *ptrBuffer, size_t blockSize);
	void clearTxSemaphore();
	SemaphoreHandle_t  txSemaphore;
	DMA_Channel_TypeDef *dmaTx;

public:
	void irqOnRxCompleate();

private:
	void dmaRxStop();
	void dmaRxStart();
	void dmaRxInit();
	size_t calculateRxFrameSize();
	DMA_Channel_TypeDef *dmaRx;

//Общее
private:
	USART_TypeDef *uart;
	uart_channel_t uart_channel;
	void uartInit();
	void uart1Init();
	void uart2Init();
	void uart3Init();
	void uart4Init();
	void initUartRegisters(USART_TypeDef * USARTx);
	void initUartPins(GPIO_TypeDef* txGPIOx, uint16_t txPin, GPIO_TypeDef* rxGPIOx, uint16_t rxPin);

};

extern ChannelUart ch1, ch2, ch3, ch4;
