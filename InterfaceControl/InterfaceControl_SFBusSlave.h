#pragma once

#include "InterfaceControlBase.h"

#include "Led.h"
#include "ncmp_layer_base.h"

class InterfaceControl_SFBusSlave : public InterfaceControlBase
{
public:
	InterfaceControl_SFBusSlave(NcmpLayerBase *_ptrNcmpLayerBase , Led * _ptrLed);
	void task();

private:
};

