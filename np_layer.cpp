#include "np_layer.h"
#include "mac_layer.h"
#include "NpFrame.h"
#include "MacFrame.h"
#include "Routing.h"
#include "debug.h"
#include "TpFrame.h"
static void NpLayer_TxTask(void *param);
static void NpLayer_RxTask(void *param);

uint16_t selfAddress = 0;

NpLayer::NpLayer(MacLayer* _ptrMacLayer, uint16_t _maxMtu, uint8_t _inderfaceId)
	:	ptrMacLayer(_ptrMacLayer),
	 	maxMtu(_maxMtu),
	 	inderfaceId(_inderfaceId),
	 	rxNcmpQueue(NULL),
	 	rxTpQueue(NULL),
	 	rxTpaQueue(NULL)

{
	txQueue = xQueueCreate(10, sizeof(MacFrame));

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

		if (dstAddress != selfAddress)	{
			Routing::instance().handleFrame(&npFrame, inderfaceId);
		}

		if ( (dstAddress == selfAddress) ||	( dstAddress == BROADCAST_ADDRESS ) ||	( dstAddress == TOP_REDIRECTION_ADDRESS ))	{

			//Нужно собирать пакет?
				//отправить в сборочный класс
				//continue;
			NpFrame_ProtocolType_t protocolType;
			protocolType = npFrame.getProtocolType();

			switch (protocolType)
			{
			case NpFrame_NCMP:
				if (putFrameToQueue(&npFrame, rxNcmpQueue) == false)	{
					npFrame.free();
				}
				break;
			case NpFrame_TPA:
				npFrame.free();
				//putFrameToQueue(&npFrame, rxTpaQueue);
				break;
			case NpFrame_TP:
				processTp(&npFrame);
				//npFrame.free();
				//putFrameToQueue(&npFrame, rxTpQueue);
				break;
			default:
				npFrame.free();
				break;
			}
		}
		else	{
			npFrame.free();
		}

	}
}

#include "Led.h"

Led greenLed(GPIOC, GPIO_Pin_6, true);

void NpLayer::processTp(NpFrame *npFrame)
{
	TpFrame tpFrame;
	tpFrame.clone(*npFrame);
	if (tpFrame.getBuffer()[0] == 0)	{
		greenLed.setState(false);
	}
	else	{
		greenLed.setState(true);
	}
	tpFrame.free();
}

bool NpLayer::putFrameToQueue(NpFrame * ptrNpFrame, QueueHandle_t queue)
{
	if (queue == NULL)	{
		return false;
	}
	BaseType_t result;
	result = xQueueSend(queue, ptrNpFrame, 50);
	assert(result == pdPASS);
	return true;
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

		bool transferResult = ptrMacLayer->send(macFrame, packetAckType_withAck);
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
