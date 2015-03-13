#include "nrf24L01Plus_HAL.h"

#include "spi.h"
#include "stm32f10x.h"

static spi_t *const ptrSpiDriver = &spi1;

static const uint32_t CS_RCC			= RCC_APB2Periph_GPIOC;
static  GPIO_TypeDef* const CS_GPIO		= GPIOC;
static const uint16_t CS_Pin			= GPIO_Pin_5;

static const uint32_t CE_RCC			= RCC_APB2Periph_GPIOA;
static  GPIO_TypeDef* const CE_GPIO		= GPIOA;
static const uint16_t CE_Pin			= GPIO_Pin_4;

/*При изменении вывода для внешнего прерывания, необходимо
отредактировать функции prv_externalInterruptInit и EXTI4_IRQHandler*/
static const uint32_t IRQ_RCC			= RCC_APB2Periph_GPIOC;
static  GPIO_TypeDef* const IRQ_GPIO	= GPIOC;
static const uint16_t IRQ_Pin			= GPIO_Pin_4;

static void prv_externalInterruptInit();

void nordic_HAL_Init()
{
	RCC_APB2PeriphClockCmd(CS_RCC | CE_RCC | IRQ_RCC, ENABLE);

	GPIO_InitTypeDef gpio;

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = CS_Pin;
	GPIO_Init(CS_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = CE_Pin;
	GPIO_Init(CE_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = IRQ_Pin;
	GPIO_Init(IRQ_GPIO, &gpio);

	nordic_HAL_ChipDeselect();
	nordic_HAL_ChipEnableLow();
	/*
	 * Максимальная допустимая частота SPI для NRF24 - 8 MHz при условии низкой емкости линии (5 pF).
	 * Наиболее подходящий предделитель - 16. Рабочая частота SPI - 4.5 MHz.
	 */
	SpiDriver_Init(ptrSpiDriver, spi_clock_mode_idle_low_1_edge, spi_first_bit_msb, spi_prescaler_16);
	prv_externalInterruptInit();
}

uint8_t nordic_HAL_SpiTransfer(uint8_t data)
{
	return SpiDriver_BlockingTransfer(ptrSpiDriver, data);
}

void nordic_HAL_ChipSelect()
{
	GPIO_WriteBit(CS_GPIO, CS_Pin, Bit_RESET);
}

void nordic_HAL_ChipDeselect()
{
	GPIO_WriteBit(CS_GPIO, CS_Pin, Bit_SET);
}

void nordic_HAL_ChipEnableLow()
{
	GPIO_WriteBit(CE_GPIO, CE_Pin, Bit_RESET);
}
void nordic_HAL_ChipEnableHigh()
{
	GPIO_WriteBit(CE_GPIO, CE_Pin, Bit_SET);
}

uint8_t nordic_HAL_GetIrqPinState()
{
	return GPIO_ReadInputDataBit(IRQ_GPIO, IRQ_Pin);
}

static void prv_externalInterruptInit()
{
	NVIC_SetPriority(EXTI4_IRQn, 14);
	NVIC_EnableIRQ(EXTI4_IRQn);

	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	EXTI->IMR &= ~EXTI_IMR_MR4;					//Включаем прерывание
	EXTI->EMR &= ~EXTI_EMR_MR4;					//Включаем евент

    AFIO->EXTICR[1] &= ~AFIO_EXTICR2_EXTI4_PC;	//Подключаем мультиплексор
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PC;

    //EXTI->RTSR |= EXTI_RTSR_TR4;
    EXTI->FTSR |= EXTI_FTSR_TR4;				//Срабатывание по спадающему фронту

    EXTI->IMR |= EXTI_IMR_MR4;					//Вклчюание прерывания по выбраннйо линии
}

void EXTI4_IRQHandler()
{
	EXTI->PR = EXTI_PR_PR4;
	nordic_HAL_OnIrqLow();
}
