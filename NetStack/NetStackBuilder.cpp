#include "NetStackBuilder.h"

#include "channel_UART.h"
#include "channel_UART_IRQ.h"
#include "MacLayerSFBus.h"

#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"

/*
 * Interface 0	-	UART2 (Slave interface)
 * Interface 1	-	UART3
 * Interface 2	-	UART1
 * Interface 3	-	UART4
 * Interface 4	-	UART5 (Interrupts)
 */

MacLayerSFBus mc0(&ch2);
MacLayerSFBus mc1(&ch3);
MacLayerSFBus mc2(&ch1);
MacLayerSFBus mc3(&ch4);
MacLayerSFBus mc4(&channelUartIrq);

NpLayer np0(&mc0, 0);
NpLayer np1(&mc1, 1);
NpLayer np2(&mc2, 2);
NpLayer np3(&mc3, 3);
NpLayer np4(&mc4, 4);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np0, &np1, &np2, &np3, &np4};

NcmpLayerSlave ncmp0(&np0, interfaceType_PointToPoint);

NcmpLayerMaster ncmp1(&np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(&np2, interfaceType_PointToPoint);
NcmpLayerMaster ncmp3(&np3, interfaceType_PointToPoint);
NcmpLayerMaster ncmp4(&np4, interfaceType_PointToPoint);

TpSocket tpSocket1;

Led led1(GPIOB, GPIO_Pin_7, false);
Led led2(GPIOB, GPIO_Pin_8, false);
Led led3(GPIOB, GPIO_Pin_9, false);

Led led4(GPIOC, GPIO_Pin_13, false);
Led led5(GPIOC, GPIO_Pin_14, false);

Pin pinInterface1_PoeStatusPin(GPIOB, GPIO_Pin_14, pinDirection_Input);
Pin pinInterface1_PoeControlPin(GPIOB, GPIO_Pin_15, pinDirection_Output);

Pin pinInterface2_PoeStatusPin(GPIOC, GPIO_Pin_9, pinDirection_Input);
Pin pinInterface2_PoeControlPin(GPIOA, GPIO_Pin_8, pinDirection_Output);

Pin pinInterface3_PoeStatusPin(GPIOA, GPIO_Pin_12, pinDirection_Input);
Pin pinInterface3_PoeControlPin(GPIOA, GPIO_Pin_15, pinDirection_Output);

Pin pinInterface4_PoeStatusPin(GPIOB, GPIO_Pin_5, pinDirection_Input);
Pin pinInterface4_PoeControlPin(GPIOB, GPIO_Pin_6, pinDirection_Output);

InterfaceControl_SFBusSlave interfaceControlSlave(
		&ncmp0,
		&led5);

InterfaceControl_SFBusMaster interfaceControlMaster1(
		&ncmp1,
		&led4,
		&pinInterface1_PoeStatusPin,
		&pinInterface1_PoeControlPin);

InterfaceControl_SFBusMaster interfaceControlMaster2(
		&ncmp2,
		&led3,
		&pinInterface2_PoeStatusPin,
		&pinInterface2_PoeControlPin);

InterfaceControl_SFBusMaster interfaceControlMaster3(
		&ncmp3,
		&led2,
		&pinInterface3_PoeStatusPin,
		&pinInterface3_PoeControlPin);

InterfaceControl_SFBusMaster interfaceControlMaster4(
		&ncmp4,
		&led1,
		&pinInterface4_PoeStatusPin,
		&pinInterface4_PoeControlPin);

void Interfaces_Init(uint16_t address)
{
	selfAddress = address;
}
