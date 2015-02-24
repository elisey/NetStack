#include "PacketAssembly.h"
#include <string.h>

PacketAssembly::PacketAssembly()
{
	int i;
	for (i = 0; i < NUM_OF_UNIQUE_NODES; ++i) {
		nodes[i].frame.alloc();
	}
}

bool PacketAssembly::insertFrame(NpFrame* ptrNpFrame)
{
	uint8_t uniqueId;
	uint16_t srcAddress;
	uniqueId = ptrNpFrame->getUniqueAssembleId();
	srcAddress = ptrNpFrame->getSrcAddress();

	int index;
	index = findNodeByUniqueIdAndSourceAddress(uniqueId, srcAddress);
	if (index == -1)	{
		index = findFreeNode();
		if (index == -1)	{
			index = findOldestNode();
		}
		initNode(index, ptrNpFrame->getTotalNumOfParts(), uniqueId, srcAddress);
	}

	if (insertPart(index, ptrNpFrame) == true)	{
		ptrNpFrame->free();
		*ptrNpFrame = nodes[index].frame;
		nodes[index].frame.alloc();
		return true;
	}
	return false;
}

int PacketAssembly::findNodeByUniqueIdAndSourceAddress(uint8_t uniqueId, uint16_t srcAddress)
{
	int i;
	for (i = 0; i < NUM_OF_UNIQUE_NODES; ++i) {
		if (nodes[i].ifFree())	{
			continue;
		}
		if (nodes[i].uniqueId == uniqueId)	{
			if (nodes[i].srcAddress == srcAddress)	{
				return i;
			}
		}
	}
	return (-1);
}

bool PacketAssembly::insertPart(int index, NpFrame* ptrNpFrame)
{
	uint8_t currentPartIndex = ptrNpFrame->getCurrentPartIndex();
	uint8_t totalNumOfParts = ptrNpFrame->getTotalNumOfParts();

	if (totalNumOfParts != nodes[index].totalNumOfParts)	{
		nodes[index].clear();
		return false;
	}

	if (currentPartIndex != nodes[index].numOfReceivedParts) {
		nodes[index].clear();
		return false;
	}

	uint8_t *ptrDst = nodes[index].frame.getPayloadPtr() + nodes[index].numOfReceivedBytes;
	uint8_t *ptrSrc = ptrNpFrame->getPayloadPtr();

	memcpy(ptrDst, ptrSrc, ptrNpFrame->getPayloadSize());

	nodes[index].numOfReceivedBytes += ptrNpFrame->getPayloadSize();


	nodes[index].numOfReceivedParts++;

	if (currentPartIndex == (totalNumOfParts - 1) )	{
		nodes[index].frame.copyHead(ptrNpFrame);

		uint8_t length = nodes[index].numOfReceivedBytes + NP_FRAME_HEAD_LENGTH;
		nodes[index].frame.getBuffer().setLenght(length);
		return true;
	}
	return false;
}

int PacketAssembly::findFreeNode()
{
	int i;
	for (i = 0; i < NUM_OF_UNIQUE_NODES; ++i) {
		if (nodes[i].ifFree())	{
			return i;
		}
	}
	return (-1);
}

int PacketAssembly::findOldestNode()
{
	static uint8_t lastOldNode = 0;
	lastOldNode++;
	if (lastOldNode >= NUM_OF_UNIQUE_NODES){
		lastOldNode = 0;
	}
	return lastOldNode;
}

void PacketAssembly::clearNode(int index)
{
	nodes[index].clear();
}

void PacketAssembly::initNode(int index, uint8_t totalNumOfParts, uint8_t uniqueId, uint16_t srcAddress)
{
	nodes[index].numOfReceivedParts = 0;
	nodes[index].totalNumOfParts = totalNumOfParts;
	nodes[index].numOfReceivedBytes = 0;
	nodes[index].uniqueId = uniqueId;
	nodes[index].srcAddress = srcAddress;
}

