#pragma  once
#include <stdio.h>
#include <stdint.h>
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG
#define DEBUG_ASSERT
#define DEBUG_PRINTF
#define LOG_PRINTF
#define DEBUG_EXIT

#ifdef DEBUG_ASSERT
	#define assert(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, (uint32_t)__LINE__))
#else
	#define assert(expr) ((void)0)
#endif

#ifdef DEBUG_PRINTF
	#define debug_printf(X, p...)  printf("\033[31m"X "\033[0m\n", ##p)
#else
	#define debug_printf		((void)0)
#endif

#ifdef LOG_PRINTF
	#define log_printf(X, p...)	printf("\033[33m" X "\033[0m", ##p)
	#define log_puts(X)			puts("\033[33m" X "\033[0m")
#else
	#define log_printf		((void)0)
	#define log_puts(X)		((void)0)
#endif

#ifdef DEBUG_EXIT
	#define debug_exit()	while(1){}
#else
	#define debug_exit()	((void)0)
#endif

#define pin0_on			GPIOC->BSRR = GPIO_Pin_8
#define pin0_off		GPIOC->BRR = GPIO_Pin_8
#define pin1_on			GPIOC->BSRR = GPIO_Pin_7
#define pin1_off		GPIOC->BRR = GPIO_Pin_7
#define pin2_on			GPIOC->BSRR = GPIO_Pin_1
#define pin2_off		GPIOC->BRR = GPIO_Pin_1
#define pin3_on			GPIOC->BSRR = GPIO_Pin_2
#define pin3_off		GPIOC->BRR = GPIO_Pin_2
#define pin4_on			GPIOC->BSRR = GPIO_Pin_3
#define pin4_off		GPIOC->BRR = GPIO_Pin_3
#define pin5_on			GPIOC->BSRR = GPIO_Pin_0
#define pin5_off		GPIOC->BRR = GPIO_Pin_0
#define pin6_on			GPIOB->BSRR = GPIO_Pin_6
#define pin6_off		GPIOB->BRR = GPIO_Pin_6
#define pin7_on			GPIOB->BSRR = GPIO_Pin_7
#define pin7_off		GPIOB->BRR = GPIO_Pin_7


extern void assert_failed(uint8_t* file, uint32_t line);
void Debug_Init();

#ifdef __cplusplus
}
#endif


