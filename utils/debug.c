#include "debug.h"

void assert_failed(uint8_t* file, uint32_t line)
{
	debug_printf("Assert failed: %s:%u\n", (char*)file, (unsigned int)line);
	while(1);
}

void Debug_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = 0b111111 ;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio);


}
