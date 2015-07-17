#pragma once

#include <stdint.h>
#include "TpFrame.h"
#include "Mutex.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "UniqueItemHandler.h"

typedef enum	{
	connectionStatus_disconnected = 1,
	connectionStatus_listen,
	connectionStatus_connected
} connectionStatus_t;

#define INPUT_RING_BUFFER_SIZE	(256)

class TpSocket {
public:
	TpSocket();
	void bind(uint8_t _selfPort);
	bool listen();
	bool connect( uint16_t dstAddress, uint8_t dstPort);
	bool close();
	void abort();
	bool write(uint8_t *buffer, unsigned int size);
	int read(uint8_t *buffer, unsigned int size, bool blocking = false);

	int getChar(TickType_t timeout);
	bool checkConnectionStatus();
	void handleRxTpFrame(TpFrame* ptrTpFrame);

	void rxTask();
	void txTask();

private:
	bool isConnected();
	bool sendConnect();
	bool sendDisconnect();
	bool sendBuffer(uint8_t *buffer, unsigned int size);
	bool transfer(TpFrame *ptrTpFrame, TpFrameType_t tpFrameType);

	void parceInDisconnectedState(TpFrame *ptrTpFrame, bool isPacketUnique);
	void parceInListenState(TpFrame *ptrTpFrame, bool isPacketUnique);
	void parceInConnectedState(TpFrame *ptrTpFrame, bool isPacketUnique);

	void clearAckQueue();
	void ackReceived(uint8_t uniqueId);
	bool waitForAck(uint8_t uniqueId, int timeout);
	bool sendAck(uint8_t _remotePort, uint16_t _remoteAddress, uint8_t uniqueId);

	void setStateDisconnected();
	void setStateListen();
	void setStateConnected(uint8_t remotePort, uint16_t remoteAddress);

	uint8_t generateNextUniqueId();

	uint8_t selfPort;
	uint8_t remotePort;
	uint16_t remoteAddress;
	connectionStatus_t connectionStatus;

	uint8_t nextUniqueId;

	QueueHandle_t rxQueue;
	QueueHandle_t ackQueue;
	xSemaphoreHandle ringBufferCountingSemaphore;
	Mutex mutex;

	uint8_t inBuffer[INPUT_RING_BUFFER_SIZE];
	volatile uint8_t wrBufferIndex;
	uint8_t rdBufferIndex;

	UniqueItemHandler<5, uint8_t> uniqueFrame;
};
