#pragma once

#include <stdint.h>
#include "PoolNode.h"

typedef enum	{
	ncmpPacket_ImSlave = 1,
	ncmpPacket_ImMaster,
	ncmpPacket_AddRoutes,
	ncmpPacket_DeleteRoutes,
	ncmpPacket_ping,
	ncmpPacket_pong,
	ncmpPacket_pingWithRoutes,
	ncmpPacket_pongWithRoutes,
}ncmpPacket_t;

class NcmpFrame	: public PoolNode
{
public:
	NcmpFrame()	 : PoolNode(10)
	{
	}

	ncmpPacket_t getPacketType()
	{
		return static_cast<ncmpPacket_t>(buffer[0]);
	}
	void setPacketType(ncmpPacket_t packetType)
	{
		buffer[0] = static_cast<uint8_t>(packetType);
	}

	void createDeleteRoutesPacket()
	{
		buffer[0] = ncmpPacket_DeleteRoutes;
		buffer[1] = 0;
		buffer.setLenght(2);
	}

	void createAddRoutesPacket()
	{
		buffer[0] = ncmpPacket_AddRoutes;
		buffer[1] = 0;
		buffer.setLenght(2);
	}

	void insertEntryToRoutesPacket(uint16_t address)
	{
		int numOfEntries = buffer[1];

		buffer[2 + (numOfEntries * 2)] = (uint8_t)( (address >> 8) & 0xff );
		buffer[2 + (numOfEntries * 2) + 1] = (uint8_t)( address & 0xff );

		numOfEntries++;
		buffer.setLenght(2 + (numOfEntries * 2));
		buffer[1] = numOfEntries;
	}

	int getNumOfEntriesInRouterPacket()
	{
		return buffer[1];
	}

	uint16_t getEntryFromRoutesPacketByIndex(int entryIndex)
	{
		uint16_t address = 0;
		address = (uint16_t)(buffer[2 + entryIndex] << 8);
		address |= (uint16_t)(buffer[2 + entryIndex + 1]);
		return address;
	}

	uint16_t getEntryFromRoutesPacket()
	{
		uint16_t address = 0;
		int numOfEntries = buffer[1];
		if (numOfEntries != 0)	{
			numOfEntries--;

			address = (uint16_t)(buffer[2 + numOfEntries] << 8);
			address |= (uint16_t)(buffer[2 + numOfEntries + 1]);
			buffer[1] = numOfEntries;
		}
		return address;
	}

private:

};
