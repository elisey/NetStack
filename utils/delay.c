#include "delay.h"
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

void Delay_RandomDelay(int maxDelayTime)
{
	uint8_t tickDelay = rand() % (maxDelayTime + 1);
	if (tickDelay != 0)	{
		vTaskDelay(tickDelay / portTICK_RATE_MS);
	}
}
