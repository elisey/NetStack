#include "Frame.h"
#include "channel_UART.h"
#include "FreeRTOS.h"
#include "task.h"

void vApplicationMallocFailedHook(void);

void sender(void *param)
{

	Channel *ptrCh = static_cast< Channel* >(param);

	Frame myFrame;
    while(1)
    {
    	myFrame.alloc();
    	myFrame.getBuffer()[0] = 12;
    	myFrame.getBuffer().setLenght(20);
    	ptrCh->send(&myFrame);
    	myFrame.free();
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
			&ch1,
			tskIDLE_PRIORITY + 1,
			NULL);
	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&ch2,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&ch3,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			sender,
			"irReceiver",
			configMINIMAL_STACK_SIZE,
			&ch4,
			tskIDLE_PRIORITY + 1,
			NULL);

	vTaskStartScheduler();


}
void vApplicationMallocFailedHook(void)
{
	while(1);
}
