#pragma once
#include <stdint.h>
#include "stm32f10x.h"

typedef enum	{
	pinDirection_Input,
	pinDirection_Output
} pinDirection_t;

class Pin {
public:
	Pin(GPIO_TypeDef* _gpio, uint16_t _pin, pinDirection_t _pinDirection, bool initState = false);

	void write(bool newState);
	void tougle();

	bool read();
private:
	GPIO_TypeDef* gpio;
	uint16_t pin;
	pinDirection_t pinDirection;
	bool state;
};
