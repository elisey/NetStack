#include "routeTable.h"
#include "debug.h"

RouterTable::RouterTable()	: defaultGate(0)
{
}

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

void RouterTable::insertRoute(uint16_t dstAddress, uint8_t interfaceId, bool isNeighbor)
{
	int freeEntryIndex;
	freeEntryIndex = getEntryIndexByDestinationAddress(dstAddress);		//попытка найти существующий марштур по нужному адресу
	if (freeEntryIndex == (-1))	{										//Если такого нет, то поиск свободной ячейки
		freeEntryIndex = getFreeEntryIndex();							//И полное копирование записи
		routerTableNodes[freeEntryIndex].address = dstAddress;
		routerTableNodes[freeEntryIndex].interfaceId = interfaceId;
		routerTableNodes[freeEntryIndex].isNeighbor = isNeighbor;
		return;
	}
	if (interfaceId != routerTableNodes[freeEntryIndex].interfaceId)	{//Если существует ячейка с таким же адресом, но другим
		routerTableNodes[freeEntryIndex].address = dstAddress;			//Интерфейсом, то полная замена ячейки
		routerTableNodes[freeEntryIndex].interfaceId = interfaceId;
		routerTableNodes[freeEntryIndex].isNeighbor = isNeighbor;
		return;
	}
	if ( (routerTableNodes[freeEntryIndex].isNeighbor == false)	&& (isNeighbor == true))	{
		routerTableNodes[freeEntryIndex].isNeighbor = true;				//Если ячейка с тами же адресом и интерфейсом,
	}																	//и если в таблице она не соседская, то обновление ее на
																		//соседскую

}

void RouterTable::deleteRoute(uint16_t dstAddress)
{
	int entryIndex;
	entryIndex = getEntryIndexByDestinationAddress(dstAddress);

	if ( entryIndex != (-1) )	{
		routerTableNodes[entryIndex].clear();
	}
}

uint16_t RouterTable::findRouteForInterface(uint8_t interfaceId)
{
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE + 1; ++i)	{
		if (routerTableNodes[i].interfaceId == interfaceId)	{
			return routerTableNodes[i].address;
		}
	}
	return 0;
}

int RouterTable::getNextRouteToPing(int prevRoute, uint8_t interfaceId)
{
/*	if (prevRoute == -1)	{
		prevRoute = 0;
	}*/

	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE + 1; ++i) {
		if (++prevRoute >= ROUTER_TABLE_SIZE)	{
			prevRoute = 0;
		}

		if (routerTableNodes[prevRoute].interfaceId == interfaceId)	{
			if ( routerTableNodes[prevRoute].isNeighbor == true )	{
				return prevRoute;
			}
		}
	}
	return (-1);
}

void RouterTable::setDefaultGate(uint16_t gate)
{
	defaultGate = gate;
}

uint16_t RouterTable::getDefaultGate()
{
	return defaultGate;
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
	return (-1);
}

int RouterTable::getEntryIndexByDestinationAddress(uint16_t dstAddress)
{
	if (dstAddress == 0)	{
		return (-1);
	}
	int i;
	for (i = 0; i < ROUTER_TABLE_SIZE; ++i) {
		if (routerTableNodes[i].address == dstAddress)	{
			return i;
		}
	}
	return (-1);
}
