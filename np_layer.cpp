#include "np_layer.h"
#include "MacFrame.h"

static void NpLayer_TxTask(void *param);
static void NpLayer_RxTask(void *param);

NpLayer::NpLayer(MacLayer* _ptrMacLayer, uint16_t _maxMtu)
	:	ptrMacLayer(_ptrMacLayer), maxMtu(_maxMtu)
{
	rxQueue = xQueueCreate(10, sizeof(MacFrame));

	xTaskCreate(
			NpLayer_TxTask,
			"NpLayer_TxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);

	xTaskCreate(
			NpLayer_RxTask,
			"NpLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

static void NpLayer_RxTask(void *param)
{
	NpLayer *ptrObj = static_cast<NpLayer*>(param);
	ptrObj->rxTask();
	while(1);
}

void NpLayer::rxTask()
{
	while(1)
	{
		MacFrame macFrame;

		bool result = ptrMacLayer->receive(macFrame, portMAX_DELAY);
		assert(result == true);

		NpFrame npFrame;
		npFrame.clone(macFrame);

		if (npFrame.getDstAddress() == selfAddress)	{

			//Нужно собирать пакет?
				//отправить в сборочный класс
				//continue;
			NpFrame_ProtocolType_t protocolType;
			protocolType = npFrame.getProtocolType();
			if (protocolType != NpFrame_NCMP)	{
				//Отправить NCMP подтверждение приема
			}
			switch (protocolType)
			{
			case NpFrame_NCMP:

				break;
			case NpFrame_TCP:

				break;
			case MpFrame_UDP:

				break;
			}
		}
		else	{

			//resend
		}




	}
}

static void NpLayer_TxTask(void *param)
{
	NpLayer *ptrObj = static_cast<NpLayer*>(param);
	ptrObj->txTask();
	while(1);
}

void NpLayer::txTask()
{
	while(1)
	{

	}
}


