#include "MacLayerSFBus.h"
#include "NetStackConfig.h"

#define SFBus_CRC_SIZE				(2u)

static void MacLayer_RxTask(void *param);

MacLayerSFBus::MacLayerSFBus(Channel* _ptrChannel)
	:	ptrChannel(_ptrChannel), nextPid(0)
{
	setMaxPayloadSize(SFBus_MAX_PAYLOAD_SIZE);

	rxQueue = xQueueCreate(SFBus_RX_MAC_QUEUE_SIZE, sizeof(MacFrame));
	ackQueue = xQueueCreate(SFBus_RX_ACK_QUEUE_SIZE, sizeof(uint8_t));

	xTaskCreate(
			MacLayer_RxTask,
			"MacLayer_RxTask",
			configMINIMAL_STACK_SIZE,
			this,
			SFBus_RX_MAC_TASK_PRIORITY,
			NULL);
}

static void MacLayer_RxTask(void *param)
{
	MacLayerSFBus *ptrObj = static_cast<MacLayerSFBus*>(param);
	ptrObj->rxTask();
	while(1);
}

void MacLayerSFBus::rxTask()
{
	while(1)
	{
		Frame frame;
		bool result;
		result = ptrChannel->receive(&frame, portMAX_DELAY);
		assert(result == true);

		MacFrame macFrame;
		macFrame.clone(frame);

		if (macFrame.checkCrc() == false)	{
			macFrame.free();
			continue;
		}

		//Убрать CRC из буфера.
		macFrame.getBuffer().setLenght( macFrame.getBuffer().getLenght() - SFBus_CRC_SIZE );

		packetAckType_t ackType = macFrame.getPacketAckType();
		uint8_t pid = macFrame.getPid();

		// Для пакетов Ack не учитываем их уникальность, обрабатываем всегда.
		// Остальные фильтруем
		if (ackType != packetAckType_Ack)	{
			if (uniqueFrame.isFrameUnique(pid) == false)	{
				macFrame.free();
				continue;
			}
			else	{
				uniqueFrame.putNewFrame(pid);
			}
		}

		switch(ackType)
		{
		case packetAckType_Ack:
			ackReceived(macFrame.getPid());
			macFrame.free();
			break;

		case packetAckType_noAck:
			handleRxPacket(&macFrame);
			break;
		case packetAckType_withAck:
			sendAck(macFrame.getPid());
			handleRxPacket(&macFrame);
			break;
		default:
			macFrame.free();
			break;
		}
	}
}

bool MacLayerSFBus::send(PoolNode *ptrPoolNode, uint16_t dstAddress)
{
	MacFrame macFrame;
	macFrame.clone(*ptrPoolNode);

	packetAckType_t ackType;
	if (SFBus_USE_ACKNOWLEDGEMENT != 0)	{
		ackType = packetAckType_withAck;
	}
	else {
		ackType = packetAckType_noAck;
	}

	macFrame.setPacketAckType(ackType);
	macFrame.setPid( getUniquePid() );
	// добавить место для CRC в буфер
	macFrame.getBuffer().setLenght( macFrame.getBuffer().getLenght() + SFBus_CRC_SIZE );
	macFrame.calculateAndSetCrc();

	bool result;

	txMutex.lock();
	result = transfer(macFrame);
	txMutex.unlock();

	return result;
}

bool MacLayerSFBus::receive(PoolNode *ptrPoolNode, unsigned int timeout)
{
/*	MacFrame macFrame;
	macFrame.clone(*ptrPoolNode);*/

	BaseType_t result;
	result = xQueueReceive(rxQueue, ptrPoolNode, timeout);
	if (result == pdPASS)	{
		return true;
	}
	return false;
}

bool MacLayerSFBus::transfer(MacFrame &macFrame)
{
	packetAckType_t ackType = macFrame.getPacketAckType();
	uint8_t packetPid = macFrame.getPid();
	Frame frame;
	frame.clone(macFrame);

	clearQueueAck();

	ptrChannel->send(&frame);
	bool transferOk = false;
	if (ackType == packetAckType_withAck)	{
		unsigned int i;
		for (i = 0; i < SFBus_RESEND_NUM - 1; ++i) {
			if (isAckReceived(packetPid, SFBus_WAIT_ACK_TIMEOUT) == true)	{
				transferOk = true;
				break;
			}

			/*
			 * Метод борьбы с коллизиями. Когда два передатчика начинают передачу отновременно,
			 * они блокируют задачу парсинга входящих сообщений, которая пытается захватить мьютекс передатчика
			 * чтобы отправить ACK (void sendAck(uint8_t pid)).
			 * Ни один из них не отвечает ACK другому, поэтому решают что их сообщение не дошло
			 * до адресата, и производят ресенды до тех пор, пока не сдедают максимально установленное
			 * их количество. В этом случае их сообщения теряются. Для борьбы с этим в случае неудачной
			 * попытки отправки сообщения передатчик отдает мьютекс и вызывает смену контекста, чтобы произошла
			 * отправка ACK в другом потоке. Предполагая что другая сторона сделала тоже самое, производится
			 * повторная проверка на наличие ACK на неудачно отправленное сообщение. Если данная ситуация имела
			 * место быть (а не отброс пакета приемной стороной из за битой контрольной суммы или переполнении входных
			 * буферов), то при повтороной проверке наличия ACK результат будет положительным.
			 */
			pin4_on;
			txMutex.unlock();
			portYIELD();
			//vTaskDelay(1);
			txMutex.lock();

			if (isAckReceived(packetPid, 0) == true)	{
				transferOk = true;
				pin4_off;
				break;
			}
			pin4_off;
			ptrChannel->send(&frame);
		}
	}
	else {
		transferOk = true;
	}
	frame.free();
	return transferOk;
}

void MacLayerSFBus::sendAck(uint8_t pid)
{
	MacFrame macFrame;
	if (macFrame.alloc(0) == false	)	{
		return;

	}
	macFrame.getBuffer().setLenght(4);
	macFrame.setPid(pid);
	macFrame.setPacketAckType(packetAckType_Ack);
	macFrame.calculateAndSetCrc();

	txMutex.lock();
	transfer(macFrame);
	txMutex.unlock();
}

void MacLayerSFBus::ackReceived(uint8_t pid)
{
	BaseType_t result;
	result = xQueueSend(ackQueue, &pid, (TickType_t)1 / portTICK_RATE_MS);
	if (result != pdPASS)	{
		//учет ошибки
	}
}

bool MacLayerSFBus::isAckReceived(uint8_t pid, unsigned int timeout)
{
	//TODO ожидание АК в течение времени, а не до первого ака

	uint8_t receivedPid;
	BaseType_t result;
	result = xQueueReceive(ackQueue, &receivedPid, (TickType_t)timeout / portTICK_RATE_MS);
	if (result == pdPASS)	{
		return (receivedPid == pid);
	}
	return false;
}

void MacLayerSFBus::clearQueueAck()
{
	bool result = pdPASS;
	uint8_t dummyPid = 0;

	do	{
		result = xQueueReceive(ackQueue, &dummyPid, (TickType_t)0);
	} while(result == pdPASS);
}

void MacLayerSFBus::handleRxPacket(MacFrame *ptrMacFrame)
{
	BaseType_t result;
	result = xQueueSend(rxQueue, ptrMacFrame, (TickType_t)50);
	assert(result == pdPASS);
}

uint8_t MacLayerSFBus::getUniquePid()
{
	return nextPid++;
}
