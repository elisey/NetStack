#pragma once
#include <stdint.h>
#include "NpFrame.h"

class Routing	{
public:
	void handleFrame(NpFrame *ptrNpFrame, uint8_t srcInterfaceId);
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
