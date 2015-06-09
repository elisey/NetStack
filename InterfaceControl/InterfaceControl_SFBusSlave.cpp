#include "InterfaceControl_SFBusSlave.h"

static void InterfaceControl_Task(void *param);

InterfaceControl_SFBusSlave::InterfaceControl_SFBusSlave(NcmpLayerBase *_ptrNcmpLayerBase , Led * _ptrLed)
	:	InterfaceControlBase(_ptrNcmpLayerBase, _ptrLed)
{
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
	InterfaceControl_SFBusSlave *ptrObj = static_cast<InterfaceControl_SFBusSlave*>(param);
	ptrObj->task();
	while(1);
}

void InterfaceControl_SFBusSlave::task()
{
	while(1)
	{
		if (ptrNcmpLayerBase->getInterfaceConnectionState() == interfaceConnectionState_Connected)	{
			interfaceState = interfaceState_Connected;
			ptrLed->setState(true);
		}
		else {
			interfaceState = interfaceState_Disconnected;
			ptrLed->setState(false);
		}

		vTaskDelay(50 / portTICK_RATE_MS);
	}
}
