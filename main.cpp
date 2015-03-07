#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "Led.h"

#include "config.h"
Led greenLed(GPIOC, GPIO_Pin_9);
Led blueLed(GPIOC, GPIO_Pin_8);

void sender(void *param)
{
	tpSocket.bind(0);
	if (tpSocket.connect(30, 1))	{
		greenLed.setState(true);
	}
    while(1)
    {
    	static uint8_t data = 0;

    	vTaskDelay(200 / portTICK_RATE_MS);

    	if (tpSocket.send(&data, 1) == true)	{
    		blueLed.tougle();
    	}
    	else {
    		greenLed.setState(false);
    		tpSocket.abort();
    		while(tpSocket.connect(30, 1) == false)
    		{
    			vTaskDelay(200 / portTICK_RATE_MS);
    		}
    		greenLed.setState(true);
		}
    	data++;
    }
}

void receiver(void *param)
{
	Led led1(GPIOC, GPIO_Pin_6);
	tpSocket.bind(1);
	tpSocket.listen();

	while(2)
	{
		int ch = tpSocket.receiveChar();
		if (ch != (-1))	{
			if ((ch % 2) == 0)	{
				led1.setState(true);
			}
			else {
				led1.setState(false);
			}
		}
		//vTaskDelay(10 / portTICK_RATE_MS);
	}
}

int main(void)
{
	Debug_Init();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &gpio);

	uint16_t value = GPIO_ReadInputData(GPIOB);
	value &= 0b11;
	value += 30;
	Interfaces_Init(value);

	__enable_irq();

	if (value == 33)	{
		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
	}
	else	{
		xTaskCreate(
					receiver,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
	}
	vTaskStartScheduler();
}

extern "C"{
void vApplicationMallocFailedHook(void)
{
	while(1);
}

void vApplicationStackOverflowHook(void)
{
	while(1);
}

}
