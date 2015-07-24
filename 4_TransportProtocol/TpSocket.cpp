#include "TpSocket.h"
#include "TpLayer.h"

#include "Routing.h"
#include "NetStackConfig.h"

#include <stdlib.h>

static void TpSocket_RxTask(void *param);

TpSocket::TpSocket(size_t _inputBufferSize)
	:	connectionStatus(connectionStatus_disconnected),
	 	nextUniqueId(0),
	 	inputBufferSize(_inputBufferSize),
	 	wrBufferIndex(0),
	 	rdBufferIndex(0)
{
	rxQueue = xQueueCreate(10, sizeof(TpFrame));
	ackQueue = xQueueCreate(3, sizeof(uint8_t));
	ringBufferCountingSemaphore = xSemaphoreCreateCounting(inputBufferSize, 0);
	xTaskCreate(
			TpSocket_RxTask,
			"TpSocket_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);

	inBuffer = (uint8_t*)(malloc(inputBufferSize));
	assert(inBuffer != NULL);
}

void TpSocket::bind(uint8_t _selfPort = 0)
{
	int result;
	result = tpLayer.registerSocket(this, _selfPort);
	assert((result != (-1)) && (result <= 0xFF));
	selfPort = result;
}

bool TpSocket::listen( )
{
	bool result = false;
	mutex.lock();
	if (connectionStatus == connectionStatus_disconnected)	{
		setStateListen();
		result = true;
	}
	mutex.unlock();
	return result;
}

bool TpSocket::connect( uint16_t _remoteAddress, uint8_t _remotePort)
{
	bool result = false;
	mutex.lock();
	setStateConnected(_remotePort, _remoteAddress);
	result = sendConnect();
	if (result == false)	{
		setStateDisconnected();
	}
	mutex.unlock();
	return result;
}

bool TpSocket::close()
{
	bool result = false;
	mutex.lock();
	if (connectionStatus == connectionStatus_connected)	{
		sendDisconnect();
		setStateDisconnected();
		result = true;
	}
	mutex.unlock();
	return result;
}

void TpSocket::abort()
{
	mutex.lock();
	setStateDisconnected();
	mutex.unlock();
}

bool TpSocket::write(uint8_t *buffer, unsigned int size)
{
	mutex.lock();
	bool result = sendBuffer(buffer, size);
	mutex.unlock();
	return result;
}

int TpSocket::read(uint8_t *buffer, unsigned int size, bool blocking)
{
	int data;
	unsigned int numOfBytes = 0;
	TickType_t timeout;

	if (blocking == true)	{
		timeout = portMAX_DELAY;
	}
	else {
		timeout = 0;
	}

	while(1)
	{
		data = getChar(timeout);
		if (data == -1)	{
			break;
		}
		buffer[numOfBytes] = (uint8_t)data;
		numOfBytes++;
		if (numOfBytes >= size)	{
			break;
		}
	}

	return numOfBytes;
}

int TpSocket::getChar(TickType_t timeout)
{
	uint8_t ch;

	BaseType_t result;
	result = xSemaphoreTake(ringBufferCountingSemaphore, timeout);
	if (result == pdFAIL)	{
		return (-1);
	}

	ch = inBuffer[rdBufferIndex];
	rdBufferIndex++;
	if (rdBufferIndex >= inputBufferSize)	{
		rdBufferIndex = 0;
	}
	return (int)( ch );
}

bool TpSocket::checkConnectionStatus()
{
	bool result = false;
	mutex.lock();

	if ( isConnected() == true)	{
		result = sendBuffer(NULL, 0);
	}
	mutex.unlock();
	return result;
}

bool TpSocket::isConnected()
{
	return (connectionStatus == connectionStatus_connected);
}

bool TpSocket::sendConnect()
{
	TpFrame tpFrame;
	tpFrame.alloc();
	tpFrame.getBuffer().setLenght(tp_FRAME_HEAD_LENGTH);
	return transfer(&tpFrame, TpFrameType_Connect);
}

bool TpSocket::sendDisconnect()
{
	TpFrame tpFrame;
	tpFrame.alloc();
	tpFrame.getBuffer().setLenght(tp_FRAME_HEAD_LENGTH);
	return transfer(&tpFrame, TpFrameType_Disconnect);
}

bool TpSocket::sendBuffer(uint8_t *buffer, unsigned int size)
{
	bool result = false;
	if (connectionStatus == connectionStatus_connected)	{

		do {
			unsigned int numOfBytesToSend = size;
			if (numOfBytesToSend > tp_MAX_TP_PAYLOAD_SIZE) {
				numOfBytesToSend = tp_MAX_TP_PAYLOAD_SIZE;
			}

			TpFrame tpFrame;
			tpFrame.alloc();
			uint8_t *dst = tpFrame.getPayloadPtr();
			memcpy( dst, buffer, numOfBytesToSend );
			tpFrame.getBuffer().setLenght(numOfBytesToSend + tp_FRAME_HEAD_LENGTH);
			result = transfer(&tpFrame, TpFrameType_Data);

			if (result == false)	{
				return false;
			}
			buffer += numOfBytesToSend;
			size -= numOfBytesToSend;
		} while (size > 0);
	}
	return result;
}

bool TpSocket::transfer(TpFrame *ptrTpFrame, TpFrameType_t tpFrameType)
{
	assert(remoteAddress != 0);

	ptrTpFrame->setSrcPort(selfPort);
	ptrTpFrame->setDstPort(remotePort);
	ptrTpFrame->setType(tpFrameType);

	uint8_t uniqueId = generateNextUniqueId();
	ptrTpFrame->setUniqueId( uniqueId );

	NpFrame npFrame;
	npFrame.clone(*ptrTpFrame);

	bool transferResult = false;
	int i;
	for (i = 0; i < tp_NUM_OF_RESEND_TRYES; ++i) {

		NpFrame npFrameToSend;
		npFrameToSend.alloc();
		npFrameToSend.copy(npFrame);

		clearAckQueue();
		bool isTransferFail = routing.send( &npFrameToSend, remoteAddress, np_MAX_TTL, NpFrame_TP );
		if (isTransferFail == false)	{
			vTaskDelay(tp_RESEND_TIMEOUT_IF_FAIL / portTICK_RATE_MS);
			continue;
		}
		bool isAckReceived;
		isAckReceived = waitForAck(uniqueId, tp_ACK_WAIT_TIMEOUT / portTICK_RATE_MS);
		if (isAckReceived == true)	{
			transferResult = true;
			break;
		}
	}
	npFrame.free();
	return transferResult;
}

void TpSocket::handleRxTpFrame(TpFrame* ptrTpFrame)
{
	BaseType_t result;
	result = xQueueSend(rxQueue, ptrTpFrame, (TickType_t)50);
	assert(result == pdPASS);
}

static void TpSocket_RxTask(void *param)
{
	TpSocket *ptrObj = static_cast<TpSocket*>(param);
	ptrObj->rxTask();
	while(1);
}

void TpSocket::rxTask()
{
	while(1)
	{
		TpFrame tpFrame;

		BaseType_t result;
		result = xQueueReceive(rxQueue, &tpFrame, portMAX_DELAY);
		assert(result == pdPASS);

		TpFrameType_t packetType = tpFrame.getType();

		if (packetType == TpFrameType_Ack)	{
			ackReceived(tpFrame.getUniqueId());
			tpFrame.free();
			continue;
		}

		uint8_t packetUniqueId = tpFrame.getUniqueId();
		bool packetIsUnique = uniqueFrame.isFrameUnique(packetUniqueId);
		if (packetIsUnique == true)	{
			uniqueFrame.putNewFrame(packetUniqueId);
		}

		if (connectionStatus == connectionStatus_listen)	{
			parceInListenState(&tpFrame, packetIsUnique);
		}
		else if (connectionStatus == connectionStatus_connected)	{
			parceInConnectedState(&tpFrame, packetIsUnique);
		}
		else if (connectionStatus == connectionStatus_disconnected)	{
			parceInDisconnectedState(&tpFrame, packetIsUnique);
		}
		tpFrame.free();
	}
}

bool TpSocket::sendAck(uint8_t _remotePort, uint16_t _remoteAddress, uint8_t uniqueId)
{
	if ((_remoteAddress == 0) || ( _remotePort == 0 ) )	{
		return false;
	}

	TpFrame tpFrame;
	tpFrame.alloc();
	tpFrame.getBuffer().setLenght(tp_FRAME_HEAD_LENGTH);

	tpFrame.setSrcPort(selfPort);
	tpFrame.setDstPort(_remotePort);
	tpFrame.setType(TpFrameType_Ack);
	tpFrame.setUniqueId( uniqueId );

	NpFrame npFrame;
	npFrame.clone(tpFrame);

	return routing.send( &npFrame, _remoteAddress, np_MAX_TTL, NpFrame_TP );
}

void TpSocket::parceInDisconnectedState(TpFrame *ptrTpFrame, bool isPacketUnique)
{

}

void TpSocket::parceInListenState(TpFrame *ptrTpFrame, bool isPacketUnique)
{
	if (ptrTpFrame->getType() == TpFrameType_Connect)	{
		uint8_t _remotePort = ptrTpFrame->getSrcPort();
		uint16_t _remoteAddress = ptrTpFrame->srcAddress;

		sendAck(_remotePort, _remoteAddress, ptrTpFrame->getUniqueId());

		if (isPacketUnique)	{
			setStateConnected(_remotePort, _remoteAddress);
		}
	}
}

void TpSocket::parceInConnectedState(TpFrame *ptrTpFrame, bool isPacketUnique)
{
	TpFrameType_t packetType = ptrTpFrame->getType();

	uint8_t _remotePort = ptrTpFrame->getSrcPort();
	uint16_t _remoteAddress = ptrTpFrame->srcAddress;
	uint8_t uniqueId = ptrTpFrame->getUniqueId();

	//TODO функциональность под вопросом
	if (packetType == TpFrameType_Connect)	{
		sendAck(_remotePort, _remoteAddress, uniqueId);
		if (isPacketUnique == true)	{
			setStateConnected(_remotePort, _remoteAddress);
		}
		return;
	}

	if (( remoteAddress != ptrTpFrame->srcAddress )	||
		( remotePort != ptrTpFrame->getSrcPort() ))	{

		//TODO SendReject
		return;
	}

	if (packetType == TpFrameType_Disconnect)	{
		sendAck(_remotePort, _remoteAddress, uniqueId);
		if (isPacketUnique == true)	{
			setStateDisconnected();
		}
		return;
	}

	if (packetType == TpFrameType_Data)	{

		sendAck(_remotePort, _remoteAddress, uniqueId);

		if (isPacketUnique == true)	{
			unsigned int size = ptrTpFrame->getPayloadSize();
			uint8_t *src = ptrTpFrame->getPayloadPtr();

			unsigned int i;
			for (i = 0; i < size; ++i) {
				waitForRxRingBufferSpaceAvailable();
				putNewByteToRxRingBuffer(src[i]);
			}
		}
		return;
	}
}

/*
 * Проверка наличия места в счетном семафоре производится в обход стандартной функциональности
 * семафора. Используется функция проверки оставшегося места в очереди, так как счетный семафор
 * реализован как очередь с нулевым размером ячеек.
 */
void TpSocket::waitForRxRingBufferSpaceAvailable()
{
	int counter = 0;
	while (uxQueueSpacesAvailable(ringBufferCountingSemaphore) == 0)	{
		counter++;
		assert(counter < 50);
		vTaskDelay(1);
	}
}

void TpSocket::putNewByteToRxRingBuffer(uint8_t data)
{
	inBuffer[wrBufferIndex] = data;
	wrBufferIndex++;
	if (wrBufferIndex >= inputBufferSize)	{
		wrBufferIndex = 0;
	}
	xSemaphoreGive(ringBufferCountingSemaphore);
}

void TpSocket::clearAckQueue()
{
	BaseType_t result = pdPASS;

	uint8_t dummy;
	while (result == pdPASS)	{
		result = xQueueReceive(ackQueue, &dummy, (TickType_t)0);
	}
}

void TpSocket::ackReceived(uint8_t uniqueId)
{
	xQueueSend(ackQueue, &uniqueId, (TickType_t)5 / portTICK_RATE_MS);
}

bool TpSocket::waitForAck(uint8_t uniqueId, int timeout)
{
	//TODO Ожидание в течении заданного промежутка времени, даже если между пришел ACK от другого пакета.

	uint8_t receivedAck = 0;

	BaseType_t result;
	result = xQueueReceive(ackQueue, &receivedAck, timeout);
	if (result == pdPASS)	{
		if (receivedAck == uniqueId)	{
			return true;
		}
	}
	return false;
}

void TpSocket::setStateDisconnected()
{
	connectionStatus = connectionStatus_disconnected;
	remotePort = 0;
	remoteAddress = 0;
}

void TpSocket::setStateListen()
{
	connectionStatus = connectionStatus_listen;
	remotePort = 0;
	remoteAddress = 0;
}

void TpSocket::setStateConnected(uint8_t _remotePort, uint16_t _remoteAddress)
{
	connectionStatus = connectionStatus_connected;
	remotePort = _remotePort;
	remoteAddress = _remoteAddress;
}

uint8_t TpSocket::generateNextUniqueId()
{
	return nextUniqueId++;
}
