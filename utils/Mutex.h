#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "debug.h"

class Mutex
{
public:
	Mutex()
	{
		mutex = xSemaphoreCreateMutex();
		assert(mutex != NULL);
	}
	void lock()
	{
		BaseType_t result = xSemaphoreTake(mutex, 50 / portTICK_RATE_MS);
		assert(result == pdPASS);
	}

	void unlock()
	{
		xSemaphoreGive(mutex);
	}

private:
	SemaphoreHandle_t mutex;
};
