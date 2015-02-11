#include "Led.h"
#include "stm32f10x.h"

Led::Led ( GPIO_TypeDef* gpio, uint16_t pin, bool state )
							: m_gpio(gpio), m_pin(pin), m_state(state)
{
	assert_param(IS_GPIO_ALL_PERIPH(m_gpio));
	assert_param(IS_GET_GPIO_PIN(m_pin));

	if (gpio == GPIOA)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	else if (gpio == GPIOB)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	else if (gpio == GPIOC)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	else if (gpio == GPIOD)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	else if (gpio == GPIOE)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	else if (gpio == GPIOF)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	else if (gpio == GPIOG)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

	GPIO_InitTypeDef sGpio;
	sGpio.GPIO_Mode = GPIO_Mode_Out_PP;
	sGpio.GPIO_Pin = pin;
	sGpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio, &sGpio);

	Led::setState(state);
}

void Led::setState ( bool newState )
{
	if (newState)	{
		m_gpio->BSRR = m_pin;
	}
	else	{
		m_gpio->BRR = m_pin;
	}
	m_state = newState;
}

void Led::tougle ()
{
	if (m_state)	{
		m_gpio->BRR = m_pin;
	}
	else	{
		m_gpio->BSRR = m_pin;
	}
	m_state = !m_state;
}
