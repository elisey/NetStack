#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "RadioMacFrame.h"
#include "mac_layer_base.h"

#define MAX_NRF_PAYLOAD_SIZE		32
#define macLayerNrfRESEND_NUM		(3)
#define macLayerNrfMAX_RESEND_DELAY	(3)
class MacLayerNrf : public MacLayerBase
{
public:
	MacLayerNrf();

	void rxTask();
	bool send( PoolNode *ptrPoolNode, uint16_t dstAddress);
	bool receive(PoolNode *ptrPoolNode, unsigned int timeout);
	void init(uint16_t selfAddress);

private:
	bool transfer(uint8_t *buffer, uint8_t size, uint16_t dstAddress);
	bool transferBroadcast(uint8_t *buffer, uint8_t size);
	bool queuePacketAndWait(uint8_t* buffer, uint8_t size);
	void wordToBuffer(uint16_t inputData, uint8_t *buffer);

	void nrfLock();
	void nrfUnlock();

	QueueHandle_t rxQueue;
};
