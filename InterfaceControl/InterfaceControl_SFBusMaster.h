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
			Pin *_ptrPoeStatusPin,
			Pin *_ptrPoeControlPin);
	void task();

	void setPoeState(bool newState);
	bool getPoeState();


private:
	bool poeState;
	Pin* ptrTxFaultPin;
	Pin* ptrRxFaultPin;
	Pin* ptrPoeStatusPin;
	Pin* ptrPoeControlPin;



};
