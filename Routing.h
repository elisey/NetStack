#pragma once
#include <stdint.h>
#include "NpFrame.h"

class Routing	{
public:
	void handleFrame(NpFrame *ptrNpFrame, uint8_t srcInterfaceId);
	bool send(NpFrame *ptrNpFrame, uint16_t dstAddess, uint8_t ttl, NpFrame_ProtocolType_t protocolType);
private:

//Singleton realisation
public:
	static Routing& instance()
	{
		static Routing theSingleInstance;
		return theSingleInstance;
	}
private:
	Routing();
	Routing(const Routing& root);
	Routing& operator=(const Routing&);
};
