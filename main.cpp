#include "Frame.h"
#include "MacFrame.h"
#include "mac_layer.h"
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
	uint16_t *address = static_cast<uint16_t*>(param);
    while(1)
    {
    	vTaskDelay(80 / portTICK_RATE_MS);
    	static uint8_t state = 0;
    	TpFrame tpFrame;
    	tpFrame.alloc();
    	tpFrame.getBuffer()[0] = state;
    	tpFrame.getBuffer().setLenght(1);

    	if (state == 0)	{
    		state = 1;
    	}
    	else {
			state = 0;
		}

    	NpFrame npFrame;
    	npFrame.clone(tpFrame);
    	Routing::instance().send(&npFrame, *address, MAX_TTL, NpFrame_TP);
    	//interfaces[1]->send(&npFrame, *address, MAX_TTL, NpFrame_TP);


    }
}

uint16_t addr1 = 30;
uint16_t addr2 = 31;
uint16_t addr3 = 32;

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);

	GPIO_InitTypeDef gpio;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &gpio);

	uint16_t value = GPIO_ReadInputData(GPIOB);
	value &= 0b11;
	selfAddress = value + 30;

	__enable_irq();
	Debug_Init();

	if (selfAddress == 33)	{
		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr1,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr2,
					tskIDLE_PRIORITY + 1,
					NULL);
		xTaskCreate(
					sender,
					"irReceiver",
					configMINIMAL_STACK_SIZE,
					&addr3,
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
