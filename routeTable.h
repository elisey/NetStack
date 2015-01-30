#pragma once
#include <stdint.h>

#define ROUTER_TABLE_SIZE	20

class RouterTableNode
{
public:
	RouterTableNode()
	:	address(0), interfaceId(0), distance(0)
	{
	}

	RouterTableNode(uint16_t _address, uint16_t _gate, uint8_t _interfaceId)
	:	address(_address), gate(_gate), interfaceId(_interfaceId)
	{
	}

	void clear()
	{
		address = 0;
		gate = 0;
		interfaceId = 0;
	}
	uint16_t address;
	uint16_t gate;
	uint8_t interfaceId;
};

class RouterTable	{
public:
	RouterTableNode& operator[](int index);
	uint8_t getInterfaceForDestinationAddress(uint16_t dstAddress);
	void insertRoute(uint16_t dstAddress, uint16_t gate, uint8_t interfaceId);
	void deleteRoute(uint16_t dstAddress);

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
