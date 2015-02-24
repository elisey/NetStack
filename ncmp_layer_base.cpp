#include "ncmp_layer_base.h"

static void NcmpLayer_Task(void *param);

NcmpLayerBase::NcmpLayerBase(NpLayer *_ptrNpLayer, interfaceType_t _interfaceType)
: 	ptrNpLayer(_ptrNpLayer),
 	interfaceType(_interfaceType)
{
	interfaceId = ptrNpLayer->getInterfaceId();
	rxQueue = xQueueCreate(10, sizeof(NpFrame));
	ptrNpLayer->setRxNcmpQueue(rxQueue);

	xTaskCreate(
			NcmpLayer_Task,
			"NcmpLayer_Task",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

static void NcmpLayer_Task(void *param)
{
	NcmpLayerBase *ptrObj = static_cast<NcmpLayerBase*>(param);
	ptrObj->task();
	while(1);
}
