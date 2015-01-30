#include "np_layer.h"
#include "mac_layer.h"
#include "NpFrame.h"
#include "MacFrame.h"
#include "Routing.h"
#include "debug.h"
static void NpLayer_TxTask(void *param);
static void NpLayer_RxTask(void *param);

NpLayer::NpLayer(MacLayer* _ptrMacLayer, uint16_t _maxMtu)
	:	ptrMacLayer(_ptrMacLayer),
	 	maxMtu(_maxMtu),
	 	rxNcmpQueue(NULL),
	 	rxTpQueue(NULL),
	 	rxTpaQueue(NULL)
{
	rxQueue = xQueueCreate(10, sizeof(MacFrame));

	xTaskCreate(
			NpLayer_TxTask,
			"NpLayer_TxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			NpLayer_RxTask,
			"NpLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

static void NpLayer_RxTask(void *param)
{
	NpLayer *ptrObj = static_cast<NpLayer*>(param);
	ptrObj->rxTask();
	while(1);
}

void NpLayer::rxTask()
{
	while(1)
	{
		MacFrame macFrame;

		bool result = ptrMacLayer->receive(macFrame, portMAX_DELAY);
		assert(result == true);

		NpFrame npFrame;
		npFrame.clone(macFrame);

		uint16_t dstAddress = npFrame.getDstAddress();

		if ( (dstAddress == selfAddress) ||	( dstAddress == BROADCAST_ADDRESS ) ||	( dstAddress == TOP_REDIRECTION_ADDRESS ))	{

			//Нужно собирать пакет?
				//отправить в сборочный класс
				//continue;
			NpFrame_ProtocolType_t protocolType;
			protocolType = npFrame.getProtocolType();

			switch (protocolType)
			{
			case NpFrame_NCMP:
				putFrameToQueue(&npFrame, rxNcmpQueue);
				break;
			case NpFrame_TPA:
				putFrameToQueue(&npFrame, rxTpaQueue);
				break;
			case NpFrame_TP:
				putFrameToQueue(&npFrame, rxTpQueue);
				break;
			default:

				break;
			}
		}
		if (dstAddress != selfAddress)	{
			Routing::instance().handleFrame(&npFrame, inderfaceId);
		}
	}
}

void NpLayer::putFrameToQueue(NpFrame * ptrNpFrame, QueueHandle_t queue)
{
	if (queue == NULL)	{
		return;
	}
	BaseType_t result;
	result = xQueueSend(queue, ptrNpFrame, portMAX_DELAY);
	assert(result == pdPASS);
}

static void NpLayer_TxTask(void *param)
{
	NpLayer *ptrObj = static_cast<NpLayer*>(param);
	ptrObj->txTask();
	while(1);
}

void NpLayer::txTask()
{
	while(1)
	{
		NpFrame npFrame;
		BaseType_t result;
		result = xQueueReceive(txQueue, &npFrame, portMAX_DELAY);
		assert(result == pdPASS);

		//Если MTU ниже размера пакета, то разбивка пакета

		MacFrame macFrame;
		macFrame.clone(npFrame);

		bool transferResult = ptrMacLayer->send(macFrame, packetAckType_Ack); //TODO отправка через гейт по умолчанию из таблицы роутов
	}
}

void NpLayer::send(NpFrame *ptrNpFrame,
		uint16_t dstAddess,
		uint8_t ttl,
		NpFrame_ProtocolType_t protocolType)
{
	ptrNpFrame->setSrcAddress( selfAddress);
	ptrNpFrame->setDstAddress(dstAddess);
	ptrNpFrame->setTtl(ttl);
	ptrNpFrame->setProtocolType(protocolType);

	BaseType_t result;
	result = xQueueSend(txQueue, ptrNpFrame, portMAX_DELAY);
	assert(result == pdPASS);
}

void NpLayer::forward(NpFrame *ptrNpFrame)
{
	BaseType_t result;
	result = xQueueSend(txQueue, ptrNpFrame, portMAX_DELAY);
	assert(result == pdPASS);
}
