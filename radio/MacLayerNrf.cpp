#include "MacLayerNrf.h"
#include "NpFrame.h"	// for BROADCAST_ADDR
#include "nrf24L01Plus.h"
#include "debug.h"

#include "delay.h"

#define ADDRESS_WIDTH	(5)		// (3 - 5)
#define CHANNEL_MHZ		(2400)	// 2400 - 2525 MHz
#define BITRATE_KBPS	(2000)	// 1000, 2000 Kbps
#define ADDRESS_FILLER	(0xC2)

uint8_t selfAddressArray[ADDRESS_WIDTH];
uint8_t broadcastAddressArray[ADDRESS_WIDTH] = {0, 0, ADDRESS_FILLER, ADDRESS_FILLER, ADDRESS_FILLER};

static SemaphoreHandle_t nrfMutex;
static SemaphoreHandle_t nrfTxSemaphore;

static void MacLayer_RxTask(void *param);

MacLayerNrf::MacLayerNrf()
{
	setMaxPayloadSize(MAX_NRF_PAYLOAD_SIZE);

	rxQueue = xQueueCreate(10, sizeof(RadioMacFrame));
	nrfMutex = xSemaphoreCreateMutex();
	nrfTxSemaphore = xSemaphoreCreateBinary();

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
	MacLayerNrf *ptrObj = static_cast<MacLayerNrf*>(param);
	ptrObj->rxTask();
	while(1);
}

void MacLayerNrf::rxTask()
{
	while(1)	{
		nrfLock();

		while( nordic_is_packet_available() )	{
			RadioMacFrame radioMacFrame;
			radioMacFrame.alloc();

			uint8_t length = nordic_get_rx_payload_width();

			/*
			 * Есть случаи,когда NRF выставляет флаг, что входящий буфер не пуст,
			 * но при попытке чтения длины этого пакета NRF отдает ноль.
			 * В таком случае помогает отчистка входящих буферов.
			 */
			if ( (length == 0) || ( length > 32 ) )	{
				nordic_flush_rx_fifo();
				//pin0_on;
			}
			else {
				radioMacFrame.getBuffer().setLenght( length );
				uint8_t* ptrBuffer = radioMacFrame.getBuffer().getDataPtr();
				nordic_read_rx_fifo(ptrBuffer, length);

				BaseType_t result;
				result = xQueueSend(rxQueue, &radioMacFrame, (TickType_t)50);
				assert(result == pdPASS);
				/*
				 * Отчистка флага прерывания только  если больше входящих
				 * пакетов нет. Так как в чипе 3-х уровневый ФИФО, и после
				 * чтения одного пакета там может остаться еще два пакета максимум.
				 */
				if (!nordic_is_packet_available()) {

				}
			}
		}
		nordic_clear_packet_available_flag();

		nrfUnlock();
		vTaskDelay(1 / portTICK_RATE_MS);
	}
}

bool MacLayerNrf::send(PoolNode *ptrPoolNode, uint16_t dstAddress)
{
	RadioMacFrame radioMacFrame;
	radioMacFrame.clone(*ptrPoolNode);


	uint8_t *ptrData = radioMacFrame.getBuffer().getDataPtr();
	uint8_t size = radioMacFrame.getBuffer().getLenght();

	bool result;

	nrfLock();

	if (dstAddress == BROADCAST_ADDRESS)	{
		result = transferBroadcast(ptrData, size);
	}
	else 	{
		result = transfer(ptrData, size, dstAddress);
	}
	nrfUnlock();
	radioMacFrame.free();
	return result;
}

bool MacLayerNrf::receive(PoolNode *ptrPoolNode, unsigned int timeout)
{
	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrPoolNode, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

void MacLayerNrf::init(uint16_t selfAddress)
{
	nrfLock();
	wordToBuffer(selfAddress, selfAddressArray);
	nordic_init((selfAddressArray), broadcastAddressArray, 32, CHANNEL_MHZ, BITRATE_KBPS, ADDRESS_WIDTH);
	nordic_standby1_to_rx();
	nrfUnlock();
}

bool MacLayerNrf::transfer(uint8_t* buffer, uint8_t size, uint16_t dstAddress)
{
	nordic_rx_to_Stanby1();
	nordic_standby1_to_tx_mode1();

	uint8_t txAddress[ADDRESS_WIDTH];
	wordToBuffer(dstAddress, txAddress);

	nordic_set_tx_address(txAddress, ADDRESS_WIDTH);
	nordic_set_rx_pipe0_addr(txAddress, ADDRESS_WIDTH);

	bool result;
	int i;
	for (i = 0; i < macLayerNrfRESEND_NUM; ++i) {
		result = queuePacketAndWait(buffer, size);
		if (result == true)	{
			break;
		}
		Delay_RandomDelay(macLayerNrfMAX_RESEND_DELAY);
	}

	nordic_set_rx_pipe0_addr(selfAddressArray, ADDRESS_WIDTH);

	nordic_standby1_to_rx();
	return result;
}

bool MacLayerNrf::transferBroadcast(uint8_t* buffer, uint8_t size)
{
	nordic_rx_to_Stanby1();
	nordic_set_auto_ack_for_pipes(false, false, false, false, false, false);
	nordic_set_tx_address(broadcastAddressArray, ADDRESS_WIDTH);

	nordic_set_auto_transmit_options(250, 0);
	nordic_standby1_to_tx_mode1();

	bool result;
	result = queuePacketAndWait(buffer, size);

	nordic_set_auto_ack_for_pipes(true, false, false, false, false, false);
    nordic_set_auto_transmit_options(500, 2);
	nordic_standby1_to_rx();
	return result;
}

bool MacLayerNrf::queuePacketAndWait(uint8_t* buffer, uint8_t size)
{
	xSemaphoreTake(nrfTxSemaphore, 0);		//Отчистка семафора
	nordic_mode1_start_send_single_packet(buffer, size);

	BaseType_t result = xSemaphoreTake(nrfTxSemaphore, 5 / portTICK_RATE_MS);
	if (result != pdPASS)	{			//TODO Случается неприем семафора

	}
	sendingState_t sendingState;
	sendingState = nordic_get_sending_state_and_clear_flag();
	nordic_mode1_finish_send_single_packet();

	return sendingState == sendingState_Ok;
}

void MacLayerNrf::wordToBuffer(uint16_t inputData, uint8_t *buffer)
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

void MacLayerNrf::nrfLock()
{
	BaseType_t result = xSemaphoreTake(nrfMutex, 50 / portTICK_RATE_MS);
	assert(result == pdPASS);
}

void MacLayerNrf::nrfUnlock()
{
	xSemaphoreGive(nrfMutex);
}

extern "C" {
	void nordic_HAL_OnIrqLow()
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		BaseType_t result;
		result = xSemaphoreGiveFromISR(nrfTxSemaphore, &xHigherPriorityTaskWoken);
		assert(result == pdPASS);
		if (xHigherPriorityTaskWoken == pdTRUE)	{
			taskYIELD();
		}
	}
}
