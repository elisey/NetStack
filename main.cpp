#include "Frame.h"
#include "MacFrame.h"
#include "MacLayerSFBus.h"
#include "NpFrame.h"
#include "ncmp_layer_master.h"
#include "ncmp_layer_slave.h"
#include "np_layer.h"
#include "channel_UART.h"
#include "FreeRTOS.h"
#include "task.h"
#include "TpFrame.h"
#include "config.h"
#include "Routing.h"
void sender(void *param)
{
	const uint8_t LENGTH = 110;
	uint16_t *address = static_cast<uint16_t*>(param);
    while(1)
    {
    	vTaskDelay(200 / portTICK_RATE_MS);
    	static uint8_t state = 0;
    	TpFrame tpFrame;
    	tpFrame.alloc();


    	int i;
    	for (i = 0; i < LENGTH; ++i) {
			(tpFrame.getBuffer())[i] = i + 10;
		}

    	if (state == 0)	{
    		state = 1;
    	}
    	else {
			state = 0;
		}

    	tpFrame.getBuffer()[0] = state;
    	tpFrame.getBuffer()[1] = LENGTH;
    	tpFrame.getBuffer().setLenght(LENGTH);

    	NpFrame npFrame;
    	npFrame.clone(tpFrame);
    	Routing::instance().send(&npFrame, *address, MAX_TTL, NpFrame_TP);
    }
}

void sender2(void *param)
{

    while(1)
    {
    	vTaskDelay(500 / portTICK_RATE_MS);

    	MacFrame macFrame;
    	macFrame.alloc();
    	macFrame.getBuffer().setLenght(4);
    	macFrame.getBuffer()[0] = 1;
    	macFrame.getBuffer()[1] = 10;

    	//rml.send(macFrame, 32);

    }
}

uint16_t addr1 = 30;
uint16_t addr2 = 33;
uint16_t addr3 = 32;

#include "PacketAssemblyTest.h"

int main(void)
{
	Debug_Init();
	pin0_on;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);
	pin0_off;
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &gpio);

	uint16_t value = GPIO_ReadInputData(GPIOB);
	value &= 0b11;
	value += 30;
	//value = 33;
	Interfaces_Init(value);

	__enable_irq();

	//PacketAssemblyTest();

	if (value == 30)	{
/*		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr1,
					tskIDLE_PRIORITY + 1,
					NULL);*/
		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr2,
					tskIDLE_PRIORITY + 1,
					NULL);
/*		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr3,
					tskIDLE_PRIORITY + 1,
					NULL);*/


	}
/*	if (selfAddress == 30)	{
		xTaskCreate(
			sender2,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 1,
			NULL);
	}*/

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
