#pragma once

#include <stdint.h>
#include "TpFrame.h"
#include "Mutex.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define NUM_OF_RESEND_TRYES	(8)

typedef enum	{
	connectionStatus_disconnected = 1,
	connectionStatus_listen,
	connectionStatus_connected
} connectionStatus_t;

class TpSocket {
public:
	TpSocket();
	void bind(uint8_t _selfPort);


	bool listen();
	bool connect( uint16_t dstAddress, uint8_t dstPort);
	bool close();
	bool abort();

	bool send(uint8_t *buffer, unsigned int size);
	bool getConnectionStatus();
	bool isConnected();
	int receiveChar();

	void handleRxTpFrame(TpFrame* ptrTpFrame);

	void rxTask();
	void txTask();


private:
	bool sendConnect();
	bool sendDisconnect();
	bool transfer(TpFrame *ptrTpFrame, TpFrameType_t tpFrameType);


	void parceInDisconnectedState(TpFrame *ptrTpFrame);
	void parceInListenState(TpFrame *ptrTpFrame);
	void parceInConnectedState(TpFrame *ptrTpFrame);

	void clearAckQueue();
	void ackReceived(uint8_t uniqueId);
	bool waitForAck(uint8_t uniqueId, int timeout);
	bool sendAck(uint8_t _remotePort, uint16_t _remoteAddress, uint8_t uniqueId);

	void setStateDisconnected();
	void setStateListen();
	void setStateConnected(uint8_t remotePort, uint16_t remoteAddress);

	uint8_t getUniqueId();
	void lock();
	void unlock();

	uint8_t selfPort;
	uint8_t remotePort;
	uint16_t remoteAddress;
	connectionStatus_t connectionStatus;

	uint8_t nextUniqueId;

	QueueHandle_t rxQueue;
	QueueHandle_t ackQueue;
	Mutex mutex;

};
