#include "InterfaceControl_SFBusMaster.h"

static void InterfaceControl_Task(void *param);

InterfaceControl_SFBusMaster::InterfaceControl_SFBusMaster(
		NcmpLayerBase *_ptrNcmpLayerBase,
		Led * _ptrLed,
		PinInput *_ptrPoeStatusPin,
		PinOutput *_ptrPoeControlPin)

:	InterfaceControlBase(_ptrNcmpLayerBase, _ptrLed),
 	poeState(true),
	ptrPoeStatusPin(_ptrPoeStatusPin),
	ptrPoeControlPin(_ptrPoeControlPin)
{
	ptrPoeControlPin->write(true);

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
	InterfaceControl_SFBusMaster *ptrObj = static_cast<InterfaceControl_SFBusMaster*>(param);
	ptrObj->task();
	while(1);
}

void InterfaceControl_SFBusMaster::task()
{
	while(1)
	{
		if(poeState == true)	{
			while (ptrPoeStatusPin->read() == false)	{
				interfaceState = interfaceState_Fault;
				//TODO log
				setPoeState(false);
				ptrLed->tougle();
				vTaskDelay(1000 / portTICK_RATE_MS);

				setPoeState(true);
				vTaskDelay(1 / portTICK_RATE_MS);
			}
		}

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

void InterfaceControl_SFBusMaster::setPoeState(bool newState)
{
	ptrPoeControlPin->write(newState);
	poeState = newState;
}

bool InterfaceControl_SFBusMaster::getPoeState()
{
	return poeState;
}
