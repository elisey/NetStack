#include "ncmp_layer_base.h"
#include "NetConfig.h"

static void NcmpLayer_Task(void *param);

NcmpLayerBase::NcmpLayerBase(NpLayer *_ptrNpLayer, interfaceType_t _interfaceType)
: 	ptrNpLayer(_ptrNpLayer),
 	interfaceType(_interfaceType),
 	interfaceState(interfaceConnectionState_Disconnected)
{
	interfaceId = ptrNpLayer->getInterfaceId();
	rxQueue = xQueueCreate(ncmp_RX_QUEUE_SIZE, sizeof(NpFrame));
	ptrNpLayer->setRxNcmpQueue(rxQueue);

	xTaskCreate(
			NcmpLayer_Task,
			"NcmpLayer_Task",
			configMINIMAL_STACK_SIZE,
			this,
			ncmp_TASK_PRIORITY,
			NULL);
}

interfaceConnectionState_t NcmpLayerBase::getInterfaceConnectionState()
{
	return interfaceState;
}

static void NcmpLayer_Task(void *param)
{
	NcmpLayerBase *ptrObj = static_cast<NcmpLayerBase*>(param);
	ptrObj->task();
	while(1);
}
