#include "TpLayer.h"
#include "TpFrame.h"
#include "debug.h"

TpLayer::TpLayer()
{

}

int TpLayer::registerSocket(TpSocket* ptrPtSocket, uint8_t port)
{
	mutex.lock();

	int resultPort = port;
	if (resultPort == 0)	{
		resultPort = getFreePortNumber();
	}

	if (isPortOccupied(resultPort) == true)	{
		resultPort = (-1);
	}
	else	{
		int index;
		index = findFreeNode();
		assert(index != (-1));
		nodes[index].ptrPtSocket = ptrPtSocket;
		nodes[index].port = resultPort;
	}
	mutex.unlock();
	return resultPort;
}

/*bool TpLayer::unregisterSocket(TpSocket *ptrPtSocket, uint8_t port)
{
	assert(port != 0);

	if (isPortOccupied(port) == false)	{
		return false;
	}

	int index;
	index = findNodeIndexByPort(port);

	if (nodes[index].ptrPtSocket == ptrPtSocket)	{
		clearNodeByIndex(index);
		return true;
	}
	return false;
}*/

void TpLayer::handleTpFrame(NpFrame *ptrNpFrame)
{
	uint16_t srcAddress = ptrNpFrame->getSrcAddress();
	TpFrame tpFrame;
	tpFrame.clone(*ptrNpFrame);
	tpFrame.srcAddress = srcAddress;

	uint8_t dstPort = tpFrame.getDstPort();

	int index;
	index = findNodeIndexByPort(dstPort);
	if (index == (-1))	{
		tpFrame.free();
		return;
	}
	if (nodes[index].ptrPtSocket != NULL)	{
		nodes[index].ptrPtSocket->handleRxTpFrame(&tpFrame);
	}
}

int TpLayer::getFreePortNumber()
{
	static int port = DYNAMIC_START_PORT_INDEX;

	int counter = 0;
	while( isPortOccupied(port) == true )	{
		port++;
		if (port == 0)	{
			port = DYNAMIC_START_PORT_INDEX;
		}
		counter++;
		if (counter >= 0xff - DYNAMIC_START_PORT_INDEX)	{
			return (-1);
		}
	}
	port++;
	return port;
}

bool TpLayer::isPortOccupied(uint8_t port)
{
	int index = findNodeIndexByPort(port);

	if (index != (-1) )	{
		return true;
	}
	return false;
}

int TpLayer::findNodeIndexByPort(uint8_t port)
{
	int i;
	for (i = 0; i < MAX_NUM_OF_SOCKETS; ++i) {
		if (nodes[i].port == port)	{
			return i;
		}
	}
	return (-1);
}

int TpLayer::findFreeNode()
{
	int i;
	for (i = 0; i < MAX_NUM_OF_SOCKETS; ++i) {
		if (nodes[i].ptrPtSocket == NULL)	{
			return i;
		}
	}
	return (-1);
}

void TpLayer::clearNodeByIndex(uint8_t index)
{
	nodes[index].clear();
}
