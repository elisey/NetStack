#include "routeTable.h"
#include "debug.h"

RouterTableNode& RouterTable::operator[](int index)
{
	return routerTableNodes[index];
}

uint8_t RouterTable::getInterfaceForDestinationAddress(uint16_t dstAddress)
{
	int entryIndex = getEntryIndexByDestinationAddress(dstAddress);

	if (entryIndex == (-1))	{
		return 0;
	}
	assert( (entryIndex < ROUTER_TABLE_SIZE) && (entryIndex >=0 ) );
	return (routerTableNodes[entryIndex].interfaceId);
}

void RouterTable::insertRoute(uint16_t dstAddress, uint8_t interfaceId, uint8_t distance)
{
	uint8_t freeEntryIndex;
	freeEntryIndex = getInterfaceForDestinationAddress(dstAddress);		//попытка найти существующий марштур по нужному адресу
	if (freeEntryIndex == 0)	{										//Если такого нет, то поиск свободной ячейки
		freeEntryIndex = getFreeEntryIndex();
	}

	routerTableNodes[freeEntryIndex].address = dstAddress;
	routerTableNodes[freeEntryIndex].interfaceId = interfaceId;
	routerTableNodes[freeEntryIndex].distance = distance;
}

void RouterTable::deleteRoute(uint16_t dstAddress)
{
	uint8_t entryIndex;
	entryIndex = getEntryIndexByDestinationAddress(dstAddress);

	if ( entryIndex != (-1) )	{
		routerTableNodes[entryIndex].clear();
	}
}

uint8_t RouterTable::getFreeEntryIndex()
{
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
		if (routerTableNodes[i].address == 0)	{		//TODO проброс адреса
			return i;
		}
	}
	assert(0);											//TODO обработка ошибки
}

int RouterTable::getEntryIndexByDestinationAddress(uint16_t dstAddress)
{
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
		if (routerTableNodes[i].address == dstAddress)	{
			return i;
		}
	}
	return (-1);
}
