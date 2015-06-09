#pragma once

#include "InterfaceControlBase.h"

#include "Led.h"
#include "ncmp_layer_base.h"

class InterfaceControl_Nrf : public InterfaceControlBase
{
public:
	InterfaceControl_Nrf(
			NcmpLayerBase *_ptrNcmpLayerBase,
			Led * _ptrLed);
	void task();

private:
};
