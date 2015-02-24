#ifndef CONFIG_H_
#define CONFIG_H_

#include "stdint.h"

#include "channel_UART.h"
#include "mac_layer.h"
#include "np_layer.h"
#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"
#include "radio_HAL.h"

//#define MASTER



#ifdef MASTER
#define NUM_OF_INTERFACES	2

#define TOP_MASTER 1

#else
#define NUM_OF_INTERFACES	1
#endif

extern NpLayer *interfaces[NUM_OF_INTERFACES];

void Interfaces_Init(uint16_t address);

#endif
