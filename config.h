#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#include "channel_UART.h"
#include "MacLayerSFBus.h"
#include "np_layer.h"
#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"
#include "MacLayerNrf.h"
#include "TpSocket.h"

#define MAX_NRF_PACKET_SIZE		32

#define MASTER

#ifdef MASTER
#define NUM_OF_INTERFACES	2
#else
#define NUM_OF_INTERFACES	1
#endif


extern NpLayer *interfaces[NUM_OF_INTERFACES];
extern TpSocket tpSocket;

void Interfaces_Init(uint16_t address);

#endif
