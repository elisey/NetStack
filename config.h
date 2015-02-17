#ifndef CONFIG_H_
#define CONFIG_H_

#include "stdint.h"

#include "channel_UART.h"
#include "mac_layer.h"
#include "np_layer.h"
#include "ncmp_layer_slave.h"
#include "ncmp_layer_master.h"
#include "radio_HAL.h"
#define NUM_OF_INTERFACES	1

extern NpLayer *interfaces[NUM_OF_INTERFACES];

//extern RadioMacLayer rml;

void Interfaces_Init(uint16_t address);

#endif
