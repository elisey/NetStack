#pragma once

#include "NpFrame.h"

#define NUM_OF_UNIQUE_NODES		(3)

class UniqueAssemblyNode
{
public:
	UniqueAssemblyNode()
	: uniqueId(0), srcAddress(0), numOfReceivedParts(0), totalNumOfParts(0), numOfReceivedBytes(0)
	{
	}

	void clear()
	{
		uniqueId = 0;
		srcAddress = 0;
		numOfReceivedParts = 0;
		totalNumOfParts = 0;
		numOfReceivedBytes = 0;
	}
	bool ifFree()
	{
		return (totalNumOfParts == 0);
	}

	uint8_t uniqueId;
	uint16_t srcAddress;
	uint8_t numOfReceivedParts;
	uint8_t totalNumOfParts;
	uint8_t numOfReceivedBytes;

	NpFrame frame;
};

class PacketAssembly
{
public:
	PacketAssembly();
	bool insertFrame(NpFrame *ptrNpFrame);

private:
	int findNodeByUniqueIdAndSourceAddress(uint8_t uniqueId, uint16_t srcAddress);
	bool insertPart(int index, NpFrame* ptrNpFrame);
	int findFreeNode();
	int findOldestNode();
	void clearNode(int index);
	void initNode(int index, uint8_t totalNumOfParts, uint8_t uniqueId, uint16_t srcAddress);

	UniqueAssemblyNode nodes[NUM_OF_UNIQUE_NODES];
};
