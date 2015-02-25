#include "np_layer.h"
#include "Routing.h"
#include "debug.h"
#include "TpFrame.h"	//TODO инклуд уберется после выделения ТП слоя
#include "routeTable.h"
#include "MacFrame.h"
static void NpLayer_TxTask(void *param);
static void NpLayer_RxTask(void *param);

uint16_t selfAddress = 0;

NpLayer::NpLayer(MacLayerBase* _ptrMacLayer, uint8_t _inderfaceId)
	:	ptrMacLayer(_ptrMacLayer),

	 	inderfaceId(_inderfaceId),
	 	rxNcmpQueue(NULL),
	 	rxTpQueue(NULL),
	 	rxTpaQueue(NULL)

{
	maxNpPayload = ptrMacLayer->getMtuSize() - NP_FRAME_HEAD_LENGTH;

	txQueue = xQueueCreate(10, sizeof(NpFrame));

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
		PoolNode poolNode(0);

		bool result = ptrMacLayer->receive(&poolNode, portMAX_DELAY);
		assert(result == true);

		NpFrame npFrame;
		npFrame.clone(poolNode);

		if (needRoutePacket(&npFrame) == true)	{
			Routing::instance().handleFrame(&npFrame, inderfaceId);
		}
		if ( needHandleOwnPacket(&npFrame) == true)	{
			handleOwnFrame(&npFrame);
		}
		else	{
			npFrame.free();
		}
	}
}

bool NpLayer::needRoutePacket(NpFrame *ptrNpFrame)
{
	uint16_t dstAddress = ptrNpFrame->getDstAddress();

	if (dstAddress != selfAddress)	{
		return true;
	}
	return false;
}

bool NpLayer::needHandleOwnPacket(NpFrame *ptrNpFrame)
{
	uint16_t dstAddress = ptrNpFrame->getDstAddress();

	if ((dstAddress == selfAddress) ||
		( dstAddress == BROADCAST_ADDRESS ) ||
		( dstAddress == TOP_REDIRECTION_ADDRESS ))
	{
		return true;
	}
	return false;
}

void NpLayer::handleOwnFrame(NpFrame *ptrNpFrame)
{
	uint16_t dstAddress = ptrNpFrame->getDstAddress();

	if (frameNeedAssemble(ptrNpFrame) == true)	{
		if (processAssembling(ptrNpFrame) == false)	{
			ptrNpFrame->free();
			return;
		}
	}
	parseOwnPacketByProtocol(ptrNpFrame);

}

bool NpLayer::frameNeedAssemble(NpFrame *ptrNpFrame)
{
	if (ptrNpFrame->getTotalNumOfParts() > 1)	{
		return true;
	}
	return false;
}

bool NpLayer::processAssembling(NpFrame *ptrNpFrame)
{
	return packetAssembly.insertFrame(ptrNpFrame);
}

void NpLayer::parseOwnPacketByProtocol(NpFrame *ptrNpFrame)
{
	NpFrame_ProtocolType_t protocolType;
	protocolType = ptrNpFrame->getProtocolType();

	switch (protocolType)
	{
	case NpFrame_NCMP:
		if (putFrameToQueue(ptrNpFrame, rxNcmpQueue) == false)	{
			ptrNpFrame->free();
		}
		break;
	case NpFrame_TPA:
		ptrNpFrame->free();
		//putFrameToQueue(&npFrame, rxTpaQueue);
		break;
	case NpFrame_TP:
		processTp(ptrNpFrame);
		//npFrame.free();
		//putFrameToQueue(&npFrame, rxTpQueue);
		break;
	default:
		ptrNpFrame->free();
		break;
	}
}

#include "Led.h"

Led greenLed(GPIOC, GPIO_Pin_9, true);

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

		uint16_t dstAddress;

		if (inderfaceId == 0)	{
			dstAddress = RouterTable::instance().getDefaultGate();
		}
		else	{
			dstAddress = npFrame.getDstAddress();
		}

		uint8_t pasketSize = npFrame.getBuffer().getLenght() - NP_FRAME_HEAD_LENGTH;

		if (pasketSize <= maxNpPayload)	{
			npFrame.setTotalNumOfParts(1);
			npFrame.setCurrentPartIndex(0);
			npFrame.setUniqueAssembleId(0);

			bool transferResult = ptrMacLayer->send(&npFrame, dstAddress);
		}
		else	{

			uint8_t uniqueId = getUniqueAssembleId();
			uint8_t currentPart = 0;
			uint8_t numOfParts = pasketSize / maxNpPayload;
			if (pasketSize % maxNpPayload != 0)	{
				numOfParts++;
			}


			uint8_t numOfTransferedBytes = 0;

			do {
				NpFrame npFramePart;
				npFramePart.alloc();
				npFramePart.copyHead( &npFrame );
				npFramePart.setTotalNumOfParts(numOfParts);
				npFramePart.setCurrentPartIndex(currentPart);
				npFramePart.setUniqueAssembleId(uniqueId);
				currentPart++;
				memcpy(npFramePart.getPayloadPtr(), npFrame.getPayloadPtr() + numOfTransferedBytes, maxNpPayload);
				npFramePart.getBuffer().setLenght(maxNpPayload + NP_FRAME_HEAD_LENGTH);
				numOfTransferedBytes += maxNpPayload;

				bool transferResult = ptrMacLayer->send(&npFramePart, dstAddress);

			} while ( (pasketSize - numOfTransferedBytes) > maxNpPayload );

			NpFrame npFramePart;
			npFramePart.alloc();
			npFramePart.copyHead( &npFrame );
			npFramePart.setTotalNumOfParts(numOfParts);
			npFramePart.setCurrentPartIndex(currentPart);
			npFramePart.setUniqueAssembleId(uniqueId);
			memcpy(npFramePart.getPayloadPtr(), npFrame.getPayloadPtr() + numOfTransferedBytes, pasketSize - numOfTransferedBytes);
			npFramePart.getBuffer().setLenght(pasketSize - numOfTransferedBytes + NP_FRAME_HEAD_LENGTH);
			bool transferResult = ptrMacLayer->send(&npFramePart, dstAddress);

			npFrame.free();
		}
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

uint8_t NpLayer::getUniqueAssembleId()
{
	static uint8_t uniqueId = 0;

	uniqueId++;
	if (uniqueId == 0)	{
		uniqueId++;
	}
	return uniqueId;
}
