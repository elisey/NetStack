#pragma once

#include "InterfaceControlBase.h"

#include "Led.h"
#include "ncmp_layer_base.h"

class InterfaceControl_SFBusMaster : public InterfaceControlBase
{
public:
	InterfaceControl_SFBusMaster(
			NcmpLayerBase *_ptrNcmpLayerBase,
			Led * _ptrLed,
			PinInput *_ptrPoeStatusPin,
			PinOutput *_ptrPoeControlPin);

	void task();

	void setPoeState(bool newState);
	bool getPoeState();

private:
	bool poeState;
	PinInput* ptrPoeStatusPin;
	PinOutput* ptrPoeControlPin;
};
