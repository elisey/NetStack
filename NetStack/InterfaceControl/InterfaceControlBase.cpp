#include "InterfaceControlBase.h"

static void InterfaceControl_Task(void *param);

InterfaceControlBase::InterfaceControlBase(NcmpLayerBase *_ptrNcmpLayerBase, Led * _ptrLed)
	:	ptrNcmpLayerBase(_ptrNcmpLayerBase),
	 	ptrLed(_ptrLed)
{
	ptrLed->setState(false);

	xTaskCreate(
			InterfaceControl_Task,
			"InterfaceControl_Task",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

static void InterfaceControl_Task(void *param)
{
	InterfaceControlBase *ptrObj = static_cast<InterfaceControlBase*>(param);
	ptrObj->task();
	while(1);
}

interfaceState_t InterfaceControlBase::getInterfaceState()
{
	return interfaceState;
}
