#pragma once

#include "TpSocket.h"
#include "NpFrame.h"
#include "Mutex.h"

#define MAX_NUM_OF_SOCKETS			(10)
#define DYNAMIC_START_PORT_INDEX	(128)

struct RegisteredSocketNode {
	RegisteredSocketNode()
	:	ptrPtSocket(NULL), port(0)
	{
	}

	void clear()
	{
		ptrPtSocket = NULL;
		port = 0;
	}
	TpSocket *ptrPtSocket;
	uint8_t port;
};


class TpLayer	{
public:

	int registerSocket(TpSocket *ptrPtSocket, uint8_t port = 0);
	//bool unregisterSocket(TpSocket *ptrPtSocket, uint8_t port);
	void handleTpFrame(NpFrame *ptrNpFrame);

private:
	int getFreePortNumber();
	bool isPortOccupied(uint8_t port);
	int findNodeIndexByPort(uint8_t port);
	int findFreeNode();
	void clearNodeByIndex(uint8_t index);

	RegisteredSocketNode nodes[MAX_NUM_OF_SOCKETS];
	Mutex mutex;

//Singleton realisation
public:
	static TpLayer& instance()
	{
		static TpLayer theSingleInstance;
		return theSingleInstance;
	}
private:
	TpLayer();
	TpLayer(const TpLayer& root);
	TpLayer& operator=(const TpLayer&);
};