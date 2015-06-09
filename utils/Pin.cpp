#include "Pin.h"

#include "debug.h"

Pin::Pin ( GPIO_TypeDef* _gpio, uint16_t _pin, pinDirection_t _pinDirection, bool initState)
							: gpio(_gpio), pin(_pin), pinDirection(_pinDirection) ,state(initState)
{
	assert_param(IS_GPIO_ALL_PERIPH(gpio));
	assert_param(IS_GET_GPIO_PIN(pin));

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
	if (pinDirection == pinDirection_Input)	{
		sGpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	}
	else	{
		sGpio.GPIO_Mode = GPIO_Mode_Out_PP;
	}
	sGpio.GPIO_Pin = pin;
	sGpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio, &sGpio);

	if (pinDirection == pinDirection_Output)	{
		Pin::write(state);
	}
}

void Pin::write ( bool newState )
{
	assert(pinDirection == pinDirection_Output);

	if (newState)	{
		gpio->BSRR = pin;
	}
	else	{
		gpio->BRR = pin;
	}
	state = newState;
}

void Pin::tougle ()
{
	assert(pinDirection == pinDirection_Output);

	if (state)	{
		gpio->BRR = pin;
	}
	else	{
		gpio->BSRR = pin;
	}
	state = !state;
}

bool Pin::read()
{
	assert(pinDirection == pinDirection_Input);

	if ((gpio->IDR & pin) != 0)	{
		return true;
	}
	else	{
		return false;
	}
}
