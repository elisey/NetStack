#pragma once
#include "Led.h"
#include "ncmp_layer_base.h"

typedef enum	{
	interfaceState_Connected,
	interfaceState_Disconnected,
	interfaceState_Fault
} interfaceState_t;

class InterfaceControlBase {
public:
	InterfaceControlBase(NcmpLayerBase *_ptrNcmpLayerBase, Led * _ptrLed);
	virtual void task() = 0;
	interfaceState_t getInterfaceState();

protected:

	interfaceState_t interfaceState;

	NcmpLayerBase *ptrNcmpLayerBase;
	Led *ptrLed;
};
