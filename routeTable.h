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

	RouterTableNode(uint16_t _address, uint8_t _interfaceId, uint8_t _distance)
	:	address(_address), interfaceId(_interfaceId), distance(_distance)
	{
	}

	uint16_t address;
	uint8_t interfaceId;
	uint8_t distance;
};

class RouterTable	{
public:
	RouterTableNode& operator[](int index)
	{
		return routerTableNodes[index];
	}

	uint8_t getRouteForDstAddress(uint16_t dstAddress)
	{
		uint8_t route = 0;
		int i;
		for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
			if (routerTableNodes[i].address == dstAddress)	{
				route = routerTableNodes[i].interfaceId;
				return route;
			}
		}
	}
private:
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
