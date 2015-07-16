#include "np_layer.h"
#include "Routing.h"
#include "debug.h"
#include "routeTable.h"
#include "MacFrame.h"
#include "TpLayer.h"
#include "NetStackConfig.h"

static void NpLayer_RxTask(void *param);

uint16_t selfAddress = 0;

NpLayer::NpLayer(MacLayerBase* _ptrMacLayer, uint8_t _inderfaceId)
	:	ptrMacLayer(_ptrMacLayer),
	 	inderfaceId(_inderfaceId),
	 	rxNcmpQueue(NULL)
{
	maxNpPayload = ptrMacLayer->getMaxPayloadSize() - np_FRAME_HEAD_SIZE;

	xTaskCreate(
			NpLayer_RxTask,
			"NpLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			np_RX_TASK_PRIORITY,
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

		#if (np_ROUTE_OTHER_PACKETS == 1)
		if (needRoutePacket(&npFrame) == true)	{
			Routing::instance().handleFrame(&npFrame, inderfaceId);
		}
		#endif
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
		( dstAddress == np_BROADCAST_ADDRESS ) ||
		( dstAddress == np_TOP_REDIRECTION_ADDRESS ))
	{
		return true;
	}
	return false;
}

void NpLayer::handleOwnFrame(NpFrame *ptrNpFrame)
{
	if (frameNeedAssemble(ptrNpFrame) == true)	{
#if (np_USE_OWN_PACKET_ASSEMBLY == 1)
		if (processAssembling(ptrNpFrame) == false)	{
			ptrNpFrame->free();
			return;
		}
#else
		ptrNpFrame->free();
		return;
#endif

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
	case NpFrame_TP:
		tpLayer.handleTpFrame(ptrNpFrame);
		break;
	default:
		ptrNpFrame->free();
		break;
	}
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

bool NpLayer::transfer(NpFrame *ptrNpFrame)
{
	bool result = false;

	txMutex.lock();
	uint16_t dstAddress = getNextHopAddress(ptrNpFrame);
	uint8_t payloadSize = ptrNpFrame->getBuffer().getLenght() - np_FRAME_HEAD_SIZE;

	if (payloadSize <= maxNpPayload)	{
		setAssemblyData(ptrNpFrame, 1, 0, 0);
		result = ptrMacLayer->send(ptrNpFrame, dstAddress);
	}
	else	{
#if (np_USE_OWN_PACKET_ASSEMBLY == 1)
		result = deassemblePacketAndSendParts(ptrNpFrame, dstAddress, payloadSize);
#else
		assert(0);
#endif
	}
	txMutex.unlock();
	return result;
}

/*
 * В зависимости от интерфейса выбирается следующий приемник, который получит это сообщение.
 * Это нужно только для NRF. Если интерфейс 0 - значит получатель сообщения может находится
 * на расстоянии в нескольких хопах, и отправка сообщения только через вышестоящего хопа,
 * который выполнит роутинг. Если интерфейс != 0, то значит получатель является прямым соседом,
 * так как по беспроводному каналу не предусмотрена многохоповость.
 */
uint16_t NpLayer::getNextHopAddress(NpFrame *ptrNpFrame)
{
	if (inderfaceId == 0)	{
		return RouterTable::instance().getDefaultGate();
	}
	else	{
		return ptrNpFrame->getDstAddress();
	}
}

bool NpLayer::deassemblePacketAndSendParts(NpFrame *ptrNpFrame, uint16_t dstAddress, uint8_t payloadSize)
{
	bool result = false;
	uint8_t uniqueId = getUniqueAssembleId();
	uint8_t currentPart = 0;
	uint8_t numOfParts = calculateNumOfParts(payloadSize);
	uint8_t numOfTransferedBytes = 0;

	for (currentPart = 0; currentPart < numOfParts; ++currentPart) {
		NpFrame npFramePart;
		npFramePart.alloc();
		npFramePart.copyHead( ptrNpFrame );
		setAssemblyData(&npFramePart, numOfParts, currentPart, uniqueId);

		uint8_t numOfBytesToCopy;
		numOfBytesToCopy = payloadSize - numOfTransferedBytes;
		if (numOfBytesToCopy > maxNpPayload)	{
			numOfBytesToCopy = maxNpPayload;
		}

		uint8_t *destination = npFramePart.getPayloadPtr();
		uint8_t *source = ptrNpFrame->getPayloadPtr() + numOfTransferedBytes;
		memcpy(destination, source, numOfBytesToCopy);
		numOfTransferedBytes += numOfBytesToCopy;

		uint8_t partPacketLength = numOfBytesToCopy + np_FRAME_HEAD_SIZE;
		npFramePart.getBuffer().setLenght( partPacketLength );

		result = ptrMacLayer->send(&npFramePart, dstAddress);
		if (result == false)	{
			break;
		}
	}
	ptrNpFrame->free();
	return result;
}

void NpLayer::setAssemblyData(NpFrame *ptrNpFrame, uint8_t totalNumOfParts, uint8_t setCurrentPartIndex, uint8_t uniqueAssebleId)
{
	ptrNpFrame->setTotalNumOfParts(totalNumOfParts);
	ptrNpFrame->setCurrentPartIndex(setCurrentPartIndex);
	ptrNpFrame->setUniqueAssembleId(uniqueAssebleId);
}

uint8_t NpLayer::calculateNumOfParts(uint8_t payloadSize)
{
	uint8_t numOfParts = payloadSize / maxNpPayload;
	if (payloadSize % maxNpPayload != 0)	{
		numOfParts++;
	}
	return numOfParts;
}

bool NpLayer::send(NpFrame *ptrNpFrame,
		uint16_t dstAddess,
		uint8_t ttl,
		NpFrame_ProtocolType_t protocolType)
{
	ptrNpFrame->setSrcAddress( selfAddress);
	ptrNpFrame->setDstAddress(dstAddess);
	ptrNpFrame->setTtl(ttl);
	ptrNpFrame->setProtocolType(protocolType);

	return transfer(ptrNpFrame);
}

bool NpLayer::forward(NpFrame *ptrNpFrame)
{
	return transfer(ptrNpFrame);
}

uint8_t NpLayer::getInterfaceId()
{
	return inderfaceId;
}

void NpLayer::setRxNcmpQueue(QueueHandle_t _rxNcmpQueue)
{
	rxNcmpQueue = _rxNcmpQueue;
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
