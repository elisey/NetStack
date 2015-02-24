#include "config.h"

#ifdef MASTER
MacLayer mc1(&ch1);
//MacLayer mc2(&ch3);
RadioHAL RadioMacLayer;

NpLayer np1(&mc1, 128, 0);
//NpLayer np2(&mc2, 128, 1);
NpLayer np3(&RadioMacLayer, 32, 2);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np3/*, &np3*/};

NcmpLayerSlave ncmp1(0, &np1, interfaceType_PointToPoint);
//NcmpLayerMaster ncmp2(1, &np2, interfaceType_PointToPoint);
NcmpLayerMaster ncmp3(1, &np3, interfaceType_Star);



#else
RadioHAL RadioMacLayer;
NpLayer np0(&RadioMacLayer, 32, 0);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np0};

NcmpLayerSlave ncmp1(0, &np0, interfaceType_Star);
#endif

void Interfaces_Init(uint16_t address)
{
	RadioMacLayer.init(address);
	selfAddress = address;
}
