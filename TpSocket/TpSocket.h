#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class TpSocket {
public:
	TpSocket();

	void task();

private:
};
