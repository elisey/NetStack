#pragma once

#include "stm32f10x.h"
#include "Pin.h"

class Led
{
public:
	Led(GPIO_TypeDef* gpio, uint16_t pin, bool state = false)
		:	pin(gpio, pin, pinDirection_Output, state)
	{
	}
	void setState(bool newState)
	{
		pin.write(newState);
	}
	void tougle()
	{
		pin.tougle();
	}
private:
	Pin pin;
};
