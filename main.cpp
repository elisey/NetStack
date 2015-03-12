#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"
#include "Led.h"

#include "config.h"
Led greenLed(GPIOC, GPIO_Pin_9);
Led blueLed(GPIOC, GPIO_Pin_8);

#define TEST_PACKET_SIZE	128

void sender1(void *param)
{
	tpSocket1.bind(0);
	if (tpSocket1.connect(30, 1))	{
		greenLed.setState(true);
	}
    while(1)
    {

    	uint8_t buffer[TEST_PACKET_SIZE];
    	int i;
    	for (i = 0; i < TEST_PACKET_SIZE; ++i) {
			buffer[i] = i;
		}

    	vTaskDelay(400 / portTICK_RATE_MS);

    	if (tpSocket1.send(buffer, TEST_PACKET_SIZE) == true)	{
    		blueLed.tougle();
    	}
    	else {
    		greenLed.setState(false);
    		tpSocket1.abort();
    		while(tpSocket1.connect(30, 1) == false)
    		{
    			vTaskDelay(400 / portTICK_RATE_MS);
    		}
    		greenLed.setState(true);
		}
    }
}

void sender2(void *param)
{
	tpSocket2.bind(0);
	if (tpSocket2.connect(30, 2))	{

	}
    while(1)
    {
    	uint8_t buffer[TEST_PACKET_SIZE];
    	int i;
    	for (i = 0; i < TEST_PACKET_SIZE; ++i) {
			buffer[i] = i;
		}

    	vTaskDelay(400 / portTICK_RATE_MS);

    	if (tpSocket2.send(buffer, TEST_PACKET_SIZE) == true)	{
    	}
    	else {
    		tpSocket2.abort();
    		while(tpSocket2.connect(30, 2) == false)
    		{
    			vTaskDelay(400 / portTICK_RATE_MS);
    		}
		}
    }
}

void sender3(void *param)
{
	tpSocket3.bind(0);
	if (tpSocket3.connect(30, 3))	{

	}
    while(1)
    {
    	uint8_t buffer[TEST_PACKET_SIZE];
    	int i;
    	for (i = 0; i < TEST_PACKET_SIZE; ++i) {
			buffer[i] = i;
		}

    	vTaskDelay(400 / portTICK_RATE_MS);

    	if (tpSocket3.send(buffer, TEST_PACKET_SIZE) == true)	{
    	}
    	else {
    		tpSocket3.abort();
    		while(tpSocket3.connect(30, 3) == false)
    		{
    			vTaskDelay(400 / portTICK_RATE_MS);
    		}
		}
    }
}

void sender4(void *param)
{
	tpSocket4.bind(0);
	if (tpSocket4.connect(30, 4))	{

	}
    while(1)
    {
    	uint8_t buffer[TEST_PACKET_SIZE];
    	int i;
    	for (i = 0; i < TEST_PACKET_SIZE; ++i) {
			buffer[i] = i;
		}

    	vTaskDelay(400 / portTICK_RATE_MS);

    	if (tpSocket4.send(buffer, TEST_PACKET_SIZE) == true)	{
    	}
    	else {
    		tpSocket4.abort();
    		while(tpSocket4.connect(30, 4) == false)
    		{
    			vTaskDelay(400 / portTICK_RATE_MS);
    		}
		}
    }
}

void receiver1(void *param)
{
	Led led1(GPIOC, GPIO_Pin_6);
	tpSocket1.bind(1);
	tpSocket1.listen();

	while(2)
	{
		uint8_t ch = tpSocket1.receiveChar();

		pin5_on;
		static uint8_t prevChar = 0;
		static int correctCharCounter = 0;
		uint8_t currentChar = ch;
		if ( (uint8_t)(prevChar+1) == currentChar)	{
			correctCharCounter++;

			if (correctCharCounter >= (TEST_PACKET_SIZE-1))	{
				correctCharCounter = 0;
				led1.tougle();
			}
		}
		else	{
			correctCharCounter = 0;
		}
		prevChar = currentChar;
		pin5_off;
	}
}

void receiver2(void *param)
{
	Led led1(GPIOC, GPIO_Pin_7);
	tpSocket2.bind(2);
	tpSocket2.listen();

	while(2)
	{
		uint8_t ch = tpSocket2.receiveChar();

		pin5_on;
		static uint8_t prevChar = 0;
		static int correctCharCounter = 0;
		uint8_t currentChar = ch;
		if ( (uint8_t)(prevChar+1) == currentChar)	{
			correctCharCounter++;

			if (correctCharCounter >= (TEST_PACKET_SIZE-1))	{
				correctCharCounter = 0;
				led1.tougle();
			}
		}
		else	{
			correctCharCounter = 0;
		}
		prevChar = currentChar;
		pin5_off;
	}
}

void receiver3(void *param)
{
	Led led1(GPIOC, GPIO_Pin_8);
	tpSocket3.bind(3);
	tpSocket3.listen();

	while(2)
	{
		uint8_t ch = tpSocket3.receiveChar();

		pin5_on;
		static uint8_t prevChar = 0;
		static int correctCharCounter = 0;
		uint8_t currentChar = ch;
		if ( (uint8_t)(prevChar+1) == currentChar)	{
			correctCharCounter++;

			if (correctCharCounter >= (TEST_PACKET_SIZE-1))	{
				correctCharCounter = 0;
				led1.tougle();
			}
		}
		else	{
			correctCharCounter = 0;
		}
		prevChar = currentChar;
		pin5_off;
	}
}

void receiver4(void *param)
{
	Led led1(GPIOC, GPIO_Pin_9);
	tpSocket4.bind(4);
	tpSocket4.listen();

	while(2)
	{
		uint8_t ch = tpSocket4.receiveChar();

		pin5_on;
		static uint8_t prevChar = 0;
		static int correctCharCounter = 0;
		uint8_t currentChar = ch;
		if ( (uint8_t)(prevChar+1) == currentChar)	{
			correctCharCounter++;

			if (correctCharCounter >= (TEST_PACKET_SIZE-1))	{
				correctCharCounter = 0;
				led1.tougle();
			}
		}
		else	{
			correctCharCounter = 0;
		}
		prevChar = currentChar;
		pin5_off;
	}
}

int main(void)
{
	Debug_Init();
	pin4_on;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);
	pin4_off;
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
					sender1,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					sender2,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					sender3,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					sender4,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
	}
	if (value == 30)	{
		xTaskCreate(
					receiver1,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					receiver2,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					receiver3,
					"irReceiver",
					200,
					NULL,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					receiver4,
					"irReceiver",
					200,
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
