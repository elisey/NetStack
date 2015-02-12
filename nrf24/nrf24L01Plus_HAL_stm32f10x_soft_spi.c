#include "nrf24L01Plus_HAL.h"
#include "stm32f10x.h"


#define NRF24_CE_GPIO	GPIOA
#define NRF24_CE_PIN	GPIO_Pin_4

#define NRF24_CSN_GPIO	GPIOC
#define NRF24_CSN_PIN	GPIO_Pin_5

#define NRF24_SCK_GPIO	GPIOA
#define NRF24_SCK_PIN	GPIO_Pin_5

#define NRF24_MOSI_GPIO	GPIOA
#define NRF24_MOSI_PIN	GPIO_Pin_7

#define NRF24_MISO_GPIO	GPIOA
#define NRF24_MISO_PIN	GPIO_Pin_6

#define NRF24_IRQ_GPIO	GPIOC
#define NRF24_IRQ_PIN	GPIO_Pin_4

#define LOW				0
#define HIGH			1

static void nrf24_sck_digitalWrite(uint8_t state);
static void nrf24_mosi_digitalWrite(uint8_t state);
static uint8_t nrf24_miso_digitalRead();

void nordic_HAL_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef gpio;

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_CE_PIN;
	GPIO_Init(NRF24_CE_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_CSN_PIN;
	GPIO_Init(NRF24_CSN_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_SCK_PIN;
	GPIO_Init(NRF24_SCK_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_MOSI_PIN;
	GPIO_Init(NRF24_MOSI_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_MISO_PIN;
	GPIO_Init(NRF24_MISO_GPIO, &gpio);

	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = NRF24_IRQ_PIN;
	GPIO_Init(NRF24_IRQ_GPIO, &gpio);
}

uint8_t nordic_HAL_SpiTransfer(uint8_t tx)
{
    uint8_t i = 0;
    uint8_t rx = 0;

    nrf24_sck_digitalWrite(LOW);

    for(i=0;i<8;i++)
    {

        if(tx & (1<<(7-i)))
        {
            nrf24_mosi_digitalWrite(HIGH);
        }
        else
        {
            nrf24_mosi_digitalWrite(LOW);
        }

        nrf24_sck_digitalWrite(HIGH);

        rx = rx << 1;
        if(nrf24_miso_digitalRead())
        {
            rx |= 0x01;
        }

        nrf24_sck_digitalWrite(LOW);

    }

    return rx;
}

void nordic_HAL_ChipSelect()
{
	GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, Bit_RESET);
}

void nordic_HAL_ChipDeselect()
{
	GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, Bit_SET);
}

void nordic_HAL_ChipEnableLow()
{
	GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, Bit_RESET);
}
void nordic_HAL_ChipEnableHigh()
{
	GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, Bit_SET);
}

/*void nrf24_ce_digitalWrite(uint8_t state)
{
	if (state) {
		GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, Bit_SET);
	}
	else {
		GPIO_WriteBit(NRF24_CE_GPIO, NRF24_CE_PIN, Bit_RESET);
	}
}

void nrf24_csn_digitalWrite(uint8_t state)
{
	if (state) {
		GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, Bit_SET);
	}
	else {
		GPIO_WriteBit(NRF24_CSN_GPIO, NRF24_CSN_PIN, Bit_RESET);
	}
}*/

uint8_t nordic_HAL_GetIrqPinState()
{
	return GPIO_ReadInputDataBit(NRF24_IRQ_GPIO, NRF24_IRQ_PIN);
}

static void nrf24_sck_digitalWrite(uint8_t state)
{
	if (state) {
		GPIO_WriteBit(NRF24_SCK_GPIO, NRF24_SCK_PIN, Bit_SET);
	}
	else {
		GPIO_WriteBit(NRF24_SCK_GPIO, NRF24_SCK_PIN, Bit_RESET);
	}
}

static void nrf24_mosi_digitalWrite(uint8_t state)
{
	if (state) {
		GPIO_WriteBit(NRF24_MOSI_GPIO, NRF24_MOSI_PIN, Bit_SET);
	}
	else {
		GPIO_WriteBit(NRF24_MOSI_GPIO, NRF24_MOSI_PIN, Bit_RESET);
	}
}

static uint8_t nrf24_miso_digitalRead()
{
	return GPIO_ReadInputDataBit(NRF24_MISO_GPIO, NRF24_MISO_PIN);
}

