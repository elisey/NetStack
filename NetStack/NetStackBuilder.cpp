#include "NetStackBuilder.h"

#include "channel_UART.h"
#include "channel_UART_IRQ.h"
#include "MacLayerSFBus.h"

#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"
#include "MacLayerNrf.h"


#ifdef MASTER
MacLayerSFBus mc1(&ch1);
//MacLayerSFBus mc2(&channelUartIrq);
MacLayerSFBus mc2(&ch3);

NpLayer np1(&mc1, 0);
NpLayer np2(&mc2, 1);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np2};

NcmpLayerSlave ncmp1(&np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(&np2, interfaceType_PointToPoint);


#endif
#ifdef BRIDGE

MacLayerSFBus mc1(&ch1);
MacLayerNrf mcRadio;

NpLayer np1(&mc1, 0);
NpLayer np2(&mcRadio, 1);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np2};

NcmpLayerSlave ncmp1(&np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(&np2, interfaceType_Star);
#endif

#ifdef SLAVE

MacLayerNrf mcRadio;

NpLayer np1(&mcRadio, 0);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1};

NcmpLayerSlave ncmp1(&np1, interfaceType_Star);

#endif


TpSocket tpSocket1;
TpSocket tpSocket2;
TpSocket tpSocket3;
TpSocket tpSocket4;
void Interfaces_Init(uint16_t address)
{
#ifdef BRIDGE
	mcRadio.init(address);
#endif
#ifdef SLAVE
	mcRadio.init(address);
#endif
	selfAddress = address;
}
