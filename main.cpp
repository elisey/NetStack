#include "Frame.h"
#include "MacFrame.h"
#include "mac_layer.h"
#include "channel_UART.h"
#include "FreeRTOS.h"
#include "task.h"

MacLayer mc1(&ch1);
MacLayer mc2(&ch2);
MacLayer mc3(&ch3);
MacLayer mc4(&ch4);

void sender(void *param)
{

	MacLayer *ptrMc = static_cast< MacLayer* >(param);

	MacFrame myFrame;
    while(1)
    {
    	myFrame.alloc();
    	myFrame.getBuffer()[2] = 12;
    	myFrame.getBuffer().setLenght(20);
    	ptrMc->send(&myFrame, packetAckType_withAck);

    	vTaskDelay(15 / portTICK_RATE_MS);
    }
}

int main(void)
{
	__enable_irq();
	Debug_Init();

	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&mc1,
			tskIDLE_PRIORITY + 1,
			NULL);
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
}

