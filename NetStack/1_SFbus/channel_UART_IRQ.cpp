#include "channel_UART_IRQ.h"

#include "stm32f10x.h"

#if (USE_UART_5 == 1)
ChannelUartIrq channelUartIrq;
#endif

ChannelUartIrq::ChannelUartIrq()
{
	uartInit();
	gpioInit();

	txSemaphore = xSemaphoreCreateBinary();

	rxFrame.alloc();
	startRx();
}

void ChannelUartIrq::transfer(Frame* ptrFrame)
{
	clearTxSemaphore();
	//transmiterEnable();					// Включение передатчика. При включении автомататически
										// посылается последовательность IDLE, которая нужна
										// для отделения пакетов друг от друга.

	ptrTxBuffer = ptrFrame->getBuffer().getDataPtr();
	txByteCounter = ptrFrame->getBuffer().getLenght();
	txeIrqEnable();
}

void ChannelUartIrq::waitForTransferCompleate()
{
	BaseType_t result;
	result = xSemaphoreTake(txSemaphore, 250 / portTICK_RATE_MS); //TODO время, обработка и прочее.
	assert(result == pdPASS);
}

void ChannelUartIrq::startRx()
{
	ptrRxBuffer = rxFrame.getBuffer().getDataPtr();
	rxByteCounter = 0;
	receiverEnable();
}

void ChannelUartIrq::stopRx()
{
	receiverDisable();
}

void ChannelUartIrq::uartInit()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5 ,ENABLE);
	USART_InitTypeDef usartStruct;

	//USART_Settings
	usartStruct.USART_BaudRate = SFBus_BAUD_RATE;
	usartStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartStruct.USART_Mode = USART_Mode_Tx;
	usartStruct.USART_Parity = USART_Parity_No;
	usartStruct.USART_StopBits = USART_StopBits_1;
	usartStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(UART5, &usartStruct);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);

	NVIC_EnableIRQ(UART5_IRQn);
	NVIC_SetPriority(UART5_IRQn, 14);

	USART_Cmd(UART5, ENABLE);
}

void ChannelUartIrq::gpioInit()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD ,ENABLE);

	GPIO_InitTypeDef gpioStruct;

	gpioStruct.GPIO_Pin = GPIO_Pin_12;
	gpioStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&gpioStruct);

	//GPIO_RX
	gpioStruct.GPIO_Pin = GPIO_Pin_2;
	gpioStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&gpioStruct);
}

void ChannelUartIrq::clearTxSemaphore()
{
	xSemaphoreTake(txSemaphore, 0);
}

void ChannelUartIrq::txeIrqEnable()
{
	USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

void ChannelUartIrq::txeIrqDisable()
{
	USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
}

void ChannelUartIrq::transmiterEnable()
{
	SET_BIT(UART5->CR1, USART_CR1_TE);
}

void ChannelUartIrq::transmiterDisable()
{
	CLEAR_BIT(UART5->CR1, USART_CR1_TE);
}

void ChannelUartIrq::irqOnTxEmpty()
{
	//TODO проверка массива на ненул.

	USART_SendData(UART5, *ptrTxBuffer);
	ptrTxBuffer++;
	txByteCounter--;
	if (txByteCounter == 0)	{
		//буфер отправлен.

		txeIrqDisable();

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(txSemaphore, &xHigherPriorityTaskWoken);
		if (xHigherPriorityTaskWoken != pdFALSE)	{
			portYIELD();
		}
	}
}

void ChannelUartIrq::receiverEnable()
{
	SET_BIT(UART5->CR1, USART_CR1_RE);
}

void ChannelUartIrq::receiverDisable()
{
	CLEAR_BIT(UART5->CR1, USART_CR1_RE);
}

void ChannelUartIrq::irqOnRxNotEmpty()
{
	if (rxByteCounter < netConfigMAX_FRAME_SIZE)	{
		ptrRxBuffer[rxByteCounter] = USART_ReceiveData(UART5);
		rxByteCounter++;
	}
}

void ChannelUartIrq::irqOnIdleReceived()
{
	stopRx();
	if ((rxByteCounter >= SFBus_MIN_PACKET_SIZE)&& (rxByteCounter <= SFBus_MAX_PACKET_SIZE))	{
		rxFrame.getBuffer().setLenght(rxByteCounter);
		/*
		 * Создаем зеркало фрейма. Пытаемся взять новый фрейм. В случае успеха бэкапнутый фрейм
		 * спихиваем вверх на обработку. Если взять новый фрейм не получилось, то он остается бех изменений
		 * Продолжаем работать с ним. А текущая принятая информация теряется, так как некуда класть новую.
		 */
		Frame tempFrame = rxFrame;
		bool result = rxFrame.allocFromIsr();
		if (result == true)	{
			Channel::handleRxPacket(&tempFrame);
		}
	}
	startRx();
}

extern "C"	{
#if (USE_UART_5 == 1)
void UART5_IRQHandler (void)
{
	if (USART_GetITStatus(UART5,USART_IT_RXNE) != RESET)	{
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);
		channelUartIrq.irqOnRxNotEmpty();
	}
	else if (USART_GetITStatus(UART5,USART_IT_TXE) != RESET)	{
		channelUartIrq.irqOnTxEmpty();
	}
	else if (USART_GetITStatus(UART5,USART_IT_IDLE) != RESET)	{
		USART_ClearITPendingBit(UART5, USART_IT_IDLE);
		UART5->SR;
		UART5->DR;
		channelUartIrq.irqOnIdleReceived();
	}
}
#endif
}
