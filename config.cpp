#include "config.h"

#ifdef MASTER
MacLayerSFBus mc1(&ch1, MAX_SFBus_PACKET_SIZE);
MacLayerSFBus mc2(&ch3, MAX_SFBus_PACKET_SIZE);

NpLayer np1(&mc1, 0);
NpLayer np3(&mc2, 1);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np3};

NcmpLayerSlave ncmp1(&np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(&np3, interfaceType_PointToPoint);

TpSocket tpSocket;

#else
MacLayerSFBus mc1(&ch1, MAX_SFBus_PACKET_SIZE);
MacLayerSFBus mc2(&ch3, MAX_SFBus_PACKET_SIZE);

NpLayer np1(&mc1, 0);
NpLayer np3(&mc2, 1);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np3};

NcmpLayerSlave ncmp1(&np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(&np3, interfaceType_PointToPoint);

TpSocket tpSocket(1);


#endif

void Interfaces_Init(uint16_t address)
{
#ifdef MASTER
	//tpSocket.connect(30, 1);
#else
	//tpSocket.listen();
#endif

	//RadioMacLayer.init(address);
	selfAddress = address;
}
