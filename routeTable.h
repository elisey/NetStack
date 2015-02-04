#pragma once
#include <stdint.h>

#define ROUTER_TABLE_SIZE	20

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
	RouterTableNode& operator[](int index);
	uint8_t getInterfaceForDestinationAddress(uint16_t dstAddress);
	void insertRoute(uint16_t dstAddress, uint8_t interfaceId, bool isNeighbor = false);
	void deleteRoute(uint16_t dstAddress);
	uint16_t findRouteForInterface(uint8_t interfaceId);
	int getNextRouteToPing(int prevRoute, uint8_t interfaceId);

private:
	uint8_t getFreeEntryIndex();
	int getEntryIndexByDestinationAddress(uint16_t dstAddress);

	RouterTableNode routerTableNodes[ROUTER_TABLE_SIZE];

	//Singleton realisation
public:
	static RouterTable& instance()
	{
		static RouterTable theSingleInstance;
		return theSingleInstance;
	}
private:
	RouterTable();
	RouterTable(const RouterTable& root);
	RouterTable& operator=(const RouterTable&);
};
