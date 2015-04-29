#include "PacketAssemblyTest.h"
#include "PacketAssembly.h"

#include "NpFrame.h"
#include "TpFrame.h"
#include "debug.h"

void PacketAssemblyTest()
{
	PacketAssembly pAssembly;

	TpFrame tpFrame;
	tpFrame.alloc();
	tpFrame.getBuffer().setLenght(10);
	int i;
	for (i = 0; i < 10; ++i) {
		(tpFrame.getBuffer().getDataPtr())[i] = i + 10;
	}

	NpFrame npFrame;
	npFrame.clone(tpFrame);

	npFrame.setDstAddress(0xA1);
	npFrame.setDstAddress(0xa2);

	npFrame.setTotalNumOfParts(2);
	npFrame.setCurrentPartIndex(0);
	npFrame.setUniqueAssembleId(55);

	if (pAssembly.insertFrame(&npFrame) == false)	{
		uint8_t size = npFrame.getBuffer().getLenght();
		uint8_t *ptrData = npFrame.getBuffer().getDataPtr();
	}
	else {
		assert(0);
	}


	npFrame.setCurrentPartIndex(1);
	if (pAssembly.insertFrame(&npFrame) == true)	{
		uint8_t size = npFrame.getBuffer().getLenght();
		uint8_t *ptrData = npFrame.getBuffer().getDataPtr();
	}
	else {
		assert(0);
	}
}
