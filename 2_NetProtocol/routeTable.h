#pragma once
#include <stdint.h>
#include "NetStackConfig.h"

class RouterTableNode
{
public:
	RouterTableNode()
	:	address(0), interfaceId(0)
	{
	}

	RouterTableNode(uint16_t _address, uint8_t _interfaceId)
	:	address(_address), interfaceId(_interfaceId), isNeighbor(false)
	{
	}

	void clear()
	{
		address = 0;
		interfaceId = 0;
		isNeighbor = false;
	}
	uint16_t address;
	uint8_t interfaceId;
	bool isNeighbor;
};

class RouterTable	{
public:
	RouterTable();
	RouterTableNode& operator[](int index);
	uint8_t getInterfaceForDestinationAddress(uint16_t dstAddress);
	void insertRoute(uint16_t dstAddress, uint8_t interfaceId, bool isNeighbor = false);
	void deleteRoute(uint16_t dstAddress);
	uint16_t findRouteForInterface(uint8_t interfaceId);
	int getNextRouteToPing(int prevRoute, uint8_t interfaceId);
	void setDefaultGate(uint16_t gate);
	uint16_t getDefaultGate();

private:
	uint8_t getFreeEntryIndex();
	int getEntryIndexByDestinationAddress(uint16_t dstAddress);

	RouterTableNode routerTableNodes[rt_ROUTER_TABLE_SIZE];
	uint16_t defaultGate;

	RouterTable(const RouterTable& root);
	RouterTable& operator=(const RouterTable&);
};

extern RouterTable routerTable;
