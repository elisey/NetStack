#include "radio_HAL.h"
#include "NpFrame.h"	// for BROADCAST_ADDR
#include "nrf24L01Plus.h"
#include "debug.h"

#define ADDRESS_WIDTH	(5)		// (3 - 5)
#define CHANNEL_MHZ		(2400)	// 2400 - 2525 MHz
#define BITRATE_KBPS	(2000)	// 1000, 2000 Kbps
#define ADDRESS_FILLER	(0xC2)

uint8_t selfAddressArray[ADDRESS_WIDTH];
uint8_t broadcastAddressArray[ADDRESS_WIDTH] = {0, 0, ADDRESS_FILLER, ADDRESS_FILLER, ADDRESS_FILLER};

static void MacLayer_RxTask(void *param);

RadioHAL::RadioHAL()
{
	rxQueue = xQueueCreate(10, sizeof(RadioMacFrame));
	nrfMutex = xSemaphoreCreateMutex();

	xTaskCreate(
			MacLayer_RxTask,
			"MacLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 2,
			NULL);
}

static void MacLayer_RxTask(void *param)
{
	RadioHAL *ptrObj = static_cast<RadioHAL*>(param);
	ptrObj->rxTask();
	while(1);
}

void RadioHAL::rxTask()
{
	while(1)	{
		xSemaphoreTake(nrfMutex, portMAX_DELAY);

		if(nordic_is_packet_available())
		{
			RadioMacFrame radioMacFrame;
			radioMacFrame.alloc();

			uint8_t length = nordic_get_rx_payload_width();
			radioMacFrame.getBuffer().setLenght( length );
			uint8_t* ptrBuffer = radioMacFrame.getBuffer().getDataPtr();
			nordic_read_rx_fifo(ptrBuffer, length);

			//поместить в очередь
			BaseType_t result;
			result = xQueueSend(rxQueue, &radioMacFrame, (TickType_t)50);
			assert(result == pdPASS);

			// Only clear the interrupt if no more receivedPacket available
			// because nordic has 3 level Rx FIFO.
			if (!nordic_is_packet_available()) {
				nordic_clear_packet_available_flag();
			}
		}
		xSemaphoreGive(nrfMutex);
		vTaskDelay(2 / portTICK_RATE_MS);
	}

	//TODO добавить семафор от прерывания

}

bool RadioHAL::send(PoolNode *ptrPoolNode, uint16_t dstAddress)
{
	RadioMacFrame radioMacFrame;
	radioMacFrame.clone(*ptrPoolNode);


	uint8_t *ptrData = radioMacFrame.getBuffer().getDataPtr();
	uint8_t size = radioMacFrame.getBuffer().getLenght();

	bool result;

	xSemaphoreTake(nrfMutex, portMAX_DELAY);

	if (dstAddress == BROADCAST_ADDRESS)	{
		result = transferBroadcast(ptrData, size);
	}
	else 	{
		result = transfer(ptrData, size, dstAddress);
	}
	xSemaphoreGive(nrfMutex);
	radioMacFrame.free();
	return result;
}

bool RadioHAL::receive(PoolNode *ptrPoolNode, unsigned int timeout)
{
/*	RadioMacFrame radioMacFrame;
	radioMacFrame.clone(*ptrPoolNode);*/

	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrPoolNode, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

void RadioHAL::init(uint16_t selfAddress)
{
	xSemaphoreTake(nrfMutex, portMAX_DELAY);
	wordToBuffer(selfAddress, selfAddressArray);

	nordic_init((selfAddressArray), broadcastAddressArray, 32, CHANNEL_MHZ, BITRATE_KBPS, ADDRESS_WIDTH);
	nordic_standby1_to_rx();
	xSemaphoreGive(nrfMutex);
}

bool RadioHAL::transfer(uint8_t* buffer, uint8_t size, uint16_t dstAddress)
{
	nordic_rx_to_Stanby1();
	nordic_standby1_to_tx_mode1();

	uint8_t txAddress[ADDRESS_WIDTH];
	wordToBuffer(dstAddress, txAddress);

	nordic_set_tx_address(txAddress, ADDRESS_WIDTH);
	nordic_set_rx_pipe0_addr(txAddress, ADDRESS_WIDTH);

	bool result;
	result = nordic_mode1_send_single_packet(buffer, size);

	nordic_set_rx_pipe0_addr(selfAddressArray, ADDRESS_WIDTH);

	nordic_standby1_to_rx();
	return result;
}

bool RadioHAL::transferBroadcast(uint8_t* buffer, uint8_t size)
{
	nordic_rx_to_Stanby1();
	nordic_set_auto_ack_for_pipes(false, false, false, false, false, false);
	nordic_set_tx_address(broadcastAddressArray, ADDRESS_WIDTH);

	nordic_set_auto_transmit_options(250, 0);
	nordic_standby1_to_tx_mode1();

	bool result;
	result = nordic_mode1_send_single_packet(buffer, size);
	nordic_set_auto_ack_for_pipes(true, false, false, false, false, false);
    nordic_set_auto_transmit_options(500, 3);
	nordic_standby1_to_rx();
	return result;
}

void RadioHAL::wordToBuffer(uint16_t inputData, uint8_t *buffer)
{
	buffer[0] = (uint8_t)(inputData & 0xFF);
	buffer[1] = (uint8_t)( (inputData >> 8) & 0xFF );
	/*
	 * Все неиспользуемые байты адреса заполняются значением 0xC2,
	 * чтобы повысить помехоустойчивость пакета.
	 */
	int i;
	for (i = 2; i < ADDRESS_WIDTH; ++i) {
		buffer[i] = ADDRESS_FILLER;
	}
}
