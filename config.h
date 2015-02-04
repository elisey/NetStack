#ifndef CONFIG_H_
#define CONFIG_H_

#include "channel_UART.h"
#include "mac_layer.h"
#include "np_layer.h"
#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"


#define NUM_OF_INTERFACES	2

extern NpLayer *interfaces[NUM_OF_INTERFACES];

#endif
