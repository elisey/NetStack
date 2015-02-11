#ifndef TEST_CLASS_H_
#define TEST_CLASS_H_

#include "stm32f10x.h"

class Led
{
public:
	Led(GPIO_TypeDef* gpio, uint16_t pin, bool state = false);
	void setState(bool newState);
	void tougle();
private:
	GPIO_TypeDef* m_gpio;
	uint16_t m_pin;
	bool m_state;
};

#endif
