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

#include "config.h"

void sender(void *param)
{
	MacLayer *ptrMc = static_cast< MacLayer* >(param);

    while(1)
    {
    	NpFrame npFrame;

    	npFrame.alloc();
    	npFrame.getBuffer()[0] = 12;
    	npFrame.getBuffer().setLenght(20);

    	MacFrame macFrame;
    	macFrame.clone(npFrame);
    	ptrMc->send(macFrame, packetAckType_withAck);

    	vTaskDelay(15 / portTICK_RATE_MS);
    }
}

int main(void)
{
	__enable_irq();
	Debug_Init();

/*	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&mc1,
			tskIDLE_PRIORITY + 1,
			NULL);*/
/*	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&mc2,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&mc3,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&mc4,
			tskIDLE_PRIORITY + 1,
			NULL);*/

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
