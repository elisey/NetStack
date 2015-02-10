#include "config.h"

MacLayer mc1(&ch1);
MacLayer mc2(&ch2);
MacLayer mc3(&ch3);
MacLayer mc4(&ch4);

NpLayer np1(&mc1, 256, 0);
NpLayer np2(&mc2, 256, 1);
NpLayer np3(&mc3, 256, 2);
NpLayer np4(&mc4, 256, 3);

NpLayer *interfaces[NUM_OF_INTERFACES] = {&np1, &np2, &np3, &np4};

NcmpLayerSlave ncmp1(0, &np1, interfaceType_PointToPoint);
NcmpLayerMaster ncmp2(1, &np2, interfaceType_PointToPoint);
NcmpLayerMaster ncmp3(2, &np3, interfaceType_PointToPoint);
NcmpLayerMaster ncmp4(3, &np4, interfaceType_PointToPoint);

