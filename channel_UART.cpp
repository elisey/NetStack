#include "channel_UART.h"

ChannelUart ch1(uart_channel_1);
ChannelUart ch2(uart_channel_2);
ChannelUart ch3(uart_channel_3);
ChannelUart ch4(uart_channel_4);

ChannelUart::ChannelUart(uart_channel_t ch_uart)
	: isTxBusyFlag(false), uart_channel(ch_uart)
{
	if (ch_uart == uart_channel_1)	{
		dmaTx = DMA1_Channel4;
		dmaRx = DMA1_Channel5;
		uart = USART1;
	}
	else if (ch_uart == uart_channel_2)	{
		dmaTx = DMA1_Channel7;
		dmaRx = DMA1_Channel6;
		uart = USART2;
	}
	else if (ch_uart == uart_channel_3)	{
		dmaTx = DMA1_Channel2;
		dmaRx = DMA1_Channel3;
		uart = USART3;
	}
	else if (ch_uart == uart_channel_4)	{
		dmaTx = DMA2_Channel5;
		dmaRx = DMA2_Channel3;
		uart = UART4;
	}

	rxFrame.alloc();

	ChannelUart::dmaTxInit();
	ChannelUart::dmaRxInit();
	ChannelUart::uartInit();
	ChannelUart::startRx();
}

void ChannelUart::transfer(Frame *frame)
{
	isTxBusyFlag = true;

	SET_BIT(uart->CR1, USART_CR1_TE);	// Включение передатчика. При включении автомататически
										// посылается последовательность IDLE, которая нужна
										// для отделения пакетов друг от друга.

	uint8_t *buffer = frame->getBuffer().getDataPtr();
	size_t size = frame->getBuffer().getLenght();
	dmaTxStart(buffer, size);
}

bool ChannelUart::isTxBusy()
{
	return isTxBusyFlag;
}

void ChannelUart::irqOnTxCompleate()
{
	DMA_Cmd(dmaTx, DISABLE);
	isTxBusyFlag = false;
}

void ChannelUart::dmaTxInit()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2, ENABLE);

	DMA_InitTypeDef dmaInitStructure;
	dmaInitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(uart->DR);
	dmaInitStructure.DMA_MemoryBaseAddr = 0;
	dmaInitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	dmaInitStructure.DMA_BufferSize = 1;
	dmaInitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dmaInitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dmaInitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dmaInitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dmaInitStructure.DMA_Mode = DMA_Mode_Normal;
	dmaInitStructure.DMA_Priority = DMA_Priority_High;
	dmaInitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmaTx, &dmaInitStructure);
}

void ChannelUart::dmaTxStart(uint8_t *ptrBuffer, size_t blockSize)
{
	assert(blockSize <= MAX_PACKET_SIZE);

	dmaTx->CMAR = (uint32_t)ptrBuffer;
	DMA_Cmd(dmaTx, DISABLE);
	dmaTx->CNDTR = blockSize;
	DMA_Cmd(dmaTx, ENABLE);
}

/*	Приемник*/

void ChannelUart::startRx()
{
	dmaRxStart();
}

void ChannelUart::stopRx()
{
	dmaRxStop();
}

void ChannelUart::irqOnRxCompleate()
{
	dmaRxStop();
	size_t rxFrameSize = calculateRxFrameSize();

	if ( (rxFrameSize >= 4) && (rxFrameSize <= MAX_PACKET_SIZE))	{
		rxFrame.getBuffer().setLenght(rxFrameSize);
		Frame tempFrame = rxFrame;

		bool result = rxFrame.allocFromIsr();
		if (result == true)	{
			Channel::handleRxPacket(&tempFrame);
		}
	}
	startRx();
}

void ChannelUart::dmaRxStop()
{
	DMA_Cmd(dmaRx, DISABLE);
}

void ChannelUart::dmaRxStart()
{
	uint8_t *dataPtr = rxFrame.getBuffer().getDataPtr();

	dmaRx->CMAR = (uint32_t)(dataPtr);
	DMA_Cmd(dmaRx, DISABLE);
	dmaRx->CNDTR = MAX_PACKET_SIZE;
	DMA_Cmd(dmaRx, ENABLE);
}

void ChannelUart::dmaRxInit()
{
	DMA_InitTypeDef dmaInitStructure;
	dmaInitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(uart->DR);
	dmaInitStructure.DMA_MemoryBaseAddr = 0;
	dmaInitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	dmaInitStructure.DMA_BufferSize = MAX_PACKET_SIZE;
	dmaInitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dmaInitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dmaInitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dmaInitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dmaInitStructure.DMA_Mode = DMA_Mode_Normal;
	dmaInitStructure.DMA_Priority = DMA_Priority_High;
	dmaInitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmaRx, &dmaInitStructure);
}

size_t ChannelUart::calculateRxFrameSize()
{
	return (MAX_PACKET_SIZE - DMA_GetCurrDataCounter(dmaRx));
}

// Общие функции инициализации

void ChannelUart::uartInit()
{
	if (uart_channel == uart_channel_1)	{
		uart1Init();
	}
	else if (uart_channel == uart_channel_2)	{
		uart2Init();
	}
	else if (uart_channel == uart_channel_3)	{
		uart3Init();
	}
	else if (uart_channel == uart_channel_4)	{
		uart4Init();
	}
}

void ChannelUart::uart1Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 ,ENABLE);
	initUartPins(GPIOA, GPIO_Pin_9, GPIOA, GPIO_Pin_10);
	initUartRegisters(USART1);
	NVIC_SetPriority(USART1_IRQn, 14);
	NVIC_EnableIRQ(USART1_IRQn);
}

void ChannelUart::uart2Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 ,ENABLE);
	initUartPins(GPIOA, GPIO_Pin_2, GPIOA, GPIO_Pin_3);
	initUartRegisters(USART2);
	NVIC_SetPriority(USART2_IRQn, 14);
	NVIC_EnableIRQ(USART2_IRQn);
}

void ChannelUart::uart3Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 ,ENABLE);
	initUartPins(GPIOB, GPIO_Pin_10, GPIOB, GPIO_Pin_11);
	initUartRegisters(USART3);
	NVIC_SetPriority(USART3_IRQn, 14);
	NVIC_EnableIRQ(USART3_IRQn);
}

void ChannelUart::uart4Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 ,ENABLE);
	initUartPins(GPIOC, GPIO_Pin_10, GPIOC, GPIO_Pin_11);
	initUartRegisters(UART4);
	NVIC_SetPriority(UART4_IRQn, 14);
	NVIC_EnableIRQ(UART4_IRQn);
}

void ChannelUart::initUartRegisters(USART_TypeDef * USARTx)
{
	USART_InitTypeDef usartStruct;

	usartStruct.USART_BaudRate = 2250000;
	usartStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartStruct.USART_Mode = USART_Mode_Rx;
	usartStruct.USART_Parity = USART_Parity_No;
	usartStruct.USART_StopBits = USART_StopBits_1;
	usartStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USARTx, &usartStruct);
	USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);
	USART_ITConfig(USARTx, USART_IT_TC, ENABLE);
	USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);
	USART_Cmd(USARTx, ENABLE);
}

void ChannelUart::initUartPins(GPIO_TypeDef* txGPIOx, uint16_t txPin, GPIO_TypeDef* rxGPIOx, uint16_t rxPin)
{
	GPIO_InitTypeDef gpioStruct;

	gpioStruct.GPIO_Pin = txPin;
	gpioStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(txGPIOx,&gpioStruct);

	gpioStruct.GPIO_Pin = rxPin;
	gpioStruct.GPIO_Mode = GPIO_Mode_IPU;
	gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(rxGPIOx,&gpioStruct);
}

extern "C"	{

void USART1_IRQHandler()
{

	if (USART_GetITStatus(USART1, USART_IT_TC) == SET)	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		CLEAR_BIT(USART1->CR1, USART_CR1_TE);
		ch1.irqOnTxCompleate();
	}
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)	{
		USART_ClearITPendingBit(USART1, USART_IT_IDLE);
		USART1->DR;
		pin0_on;
		ch1.irqOnRxCompleate();
		pin0_off;
	}

}

void USART2_IRQHandler()
{
	if (USART_GetITStatus(USART2, USART_IT_TC) == SET)	{
		USART_ClearITPendingBit(USART2, USART_IT_TC);
		CLEAR_BIT(USART2->CR1, USART_CR1_TE);
		ch2.irqOnTxCompleate();
	}
	else if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)	{
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);
		USART2->DR;
		pin1_on;
		ch2.irqOnRxCompleate();
		pin1_off;
	}
}

void USART3_IRQHandler()
{
	if (USART_GetITStatus(USART3, USART_IT_TC) == SET)	{
		USART_ClearITPendingBit(USART3, USART_IT_TC);
		CLEAR_BIT(USART3->CR1, USART_CR1_TE);
		ch3.irqOnTxCompleate();
	}
	else if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET)	{
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
		USART3->DR;
		pin3_on;
		ch3.irqOnRxCompleate();
		pin3_off;
	}
}

void UART4_IRQHandler()
{
	if (USART_GetITStatus(UART4, USART_IT_TC) == SET)	{
		USART_ClearITPendingBit(UART4, USART_IT_TC);
		CLEAR_BIT(UART4->CR1, USART_CR1_TE);
		ch4.irqOnTxCompleate();
	}
	else if (USART_GetITStatus(UART4, USART_IT_IDLE) == SET)	{
		USART_ClearITPendingBit(UART4, USART_IT_IDLE);
		UART4->DR;
		ch4.irqOnRxCompleate();
	}
}
}