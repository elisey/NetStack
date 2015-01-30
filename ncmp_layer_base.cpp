#include "ncmp_layer_base.h"

static void NcmpLayer_Task(void *param);

NcmpLayerBase::NcmpLayerBase(uint8_t _interfaceId, NpLayer *_ptrNpLayer)
:	interfaceId(_interfaceId),
 	ptrNpLayer(_ptrNpLayer)
{
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
	NcmpLayer *ptrObj = static_cast<NcmpLayer*>(param);
	ptrObj->task();
	while(1);
}
