#include "ncmp_layer.h"
#include "NpFrame.h"
#include "NcmpFrame.h"
static void NcmpLayer_Task(void *param);

NcmpLayer::NcmpLayer(uint8_t _interfaceId , NpLayer *_ptrNpLayer)
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

void NcmpLayer::task()
{
	if (interfaceId == 0)	{
		taskMainInterface();
	}
	else	{
		taskSlaveInterface();
	}
}

void NcmpLayer::taskMainInterface()
{
	while(1)
	{
		uint16_t currentMaster = 0;

		if (currentMaster == 0)	{
			sendImHere();

			NpFrame npFrame;
			BaseType_t result = xQueueReceive(rxQueue, &npFrame, 10);
			if (result == pdPASS)	{
				currentMaster = npFrame.getSrcAddress();

				NcmpFrame ncmpFrame;
				ncmpFrame.clone(npFrame);

				if (ncmpFrame.getPacketType() == ncmpPacket_giveRt)	{
					sendRt(currentMaster);
					continue;
				}

				if (ncmpFrame.getPacketType() == ncmpPacket_ping)	{
					sendPong(currentMaster);
					continue;
				}
				currentMaster = 0;
			}
		}
		else	{
			NpFrame npFrame;
			BaseType_t result = xQueueReceive(rxQueue, &npFrame, 10);
			if (result == pdPASS)	{
				if (npFrame.getSrcAddress() == currentMaster)	{
					NcmpFrame ncmpFrame;
					ncmpFrame.clone(npFrame);

					if (ncmpFrame.getPacketType() == ncmpPacket_giveRt)	{
						sendRt(currentMaster);
						continue;
					}

					if (ncmpFrame.getPacketType() == ncmpPacket_ping)	{
						sendPong(currentMaster);
						continue;
					}
				}
			}
			currentMaster == 0;
		}
	}
}

void NcmpLayer::sendPing(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_ping);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayer::sendPong(uint16_t dstAddress)
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_pong);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, dstAddress, 1, NpFrame_NCMP);
}

void NcmpLayer::sendImHere()
{
	NcmpFrame ncmpFrame;
	ncmpFrame.alloc();
	ncmpFrame.getBuffer().setLenght(1);

	ncmpFrame.setPacketType(ncmpPacket_im_here);
	NpFrame npFrame;
	npFrame.clone(ncmpFrame);
	ptrNpLayer->send(&npFrame, BROADCAST_ADDRESS, 1, NpFrame_NCMP);
}

void NcmpLayer::sendRt(uint16_t dstAddress)		//TODO Добавить отправку таблицы
{

}



void NcmpLayer::taskSlaveInterface()
{
	while(1)
	{
		// Взять из таблицы следующий слейв для текущего интерфейса
		// PingSend();

		// В течении времени ждать входящего сообщения:
		// Если входящее сообщение Pong:
		// 		continue

		// Если входящее сообщение Im here:
			// send giveRT;

		// Если входящее сообщение RT:
		// 		Добавить в таблицу  новые пути (Отправить RT выше)

		// Неудача. Сделать еще несколько попыток.
	}
}


