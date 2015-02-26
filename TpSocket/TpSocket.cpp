#include "TpSocket.h"

static void TpSocket_Task(void *param);

TpSocket::TpSocket()
{
	xTaskCreate(
			TpSocket_Task,
			"TpSocket_Task",
			configMINIMAL_STACK_SIZE,
			this,
			tskIDLE_PRIORITY + 1,
			NULL);
}

static void TpSocket_Task(void *param)
{
	TpSocket *ptrObj = static_cast<TpSocket*>(param);
	ptrObj->task();
	while(1);
}

void TpSocket::task()
{

}
