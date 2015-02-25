#include "config.h"

#ifdef MASTER
MacLayer mc1(&ch1, MAX_RS_PACKET_SIZE);
//MacLayer mc2(&ch3, MAX_RS_PACKET_SIZE);
RadioHAL RadioMacLayer(MAX_NRF_PACKET_SIZE);

NpLayer np1(&mc1, 0);
//NpLayer np2(&mc2, 1);
NpLayer np3(&RadioMacLayer, 1);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np3/*, &np3*/};

NcmpLayerSlave ncmp1(&np1, interfaceType_PointToPoint);
//NcmpLayerMaster ncmp2(&np2, interfaceType_PointToPoint);
NcmpLayerMaster ncmp3(&np3, interfaceType_Star);

#else
RadioHAL RadioMacLayer(MAX_NRF_PACKET_SIZE);
NpLayer np0(&RadioMacLayer, 0);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np0};

NcmpLayerSlave ncmp1(&np0, interfaceType_Star);
#endif

void Interfaces_Init(uint16_t address)
{
	RadioMacLayer.init(address);
	selfAddress = address;
}
