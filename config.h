#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#include "np_layer.h"
#include "TpSocket.h"

#define DEVICE_TYPE	2

#if (DEVICE_TYPE == 1)
#define MASTER
#elif (DEVICE_TYPE == 2)
#define BRIDGE
#elif (DEVICE_TYPE == 3)
#define SLAVE
#endif

#ifdef MASTER
#define NUM_OF_INTERFACES	2
#endif
#ifdef BRIDGE
#define NUM_OF_INTERFACES	2
#endif
#ifdef SLAVE
#define NUM_OF_INTERFACES	1
#endif


extern NpLayer *interfaces[NUM_OF_INTERFACES];
extern TpSocket tpSocket1;
extern TpSocket tpSocket2;
extern TpSocket tpSocket3;
extern TpSocket tpSocket4;
void Interfaces_Init(uint16_t address);

#endif
