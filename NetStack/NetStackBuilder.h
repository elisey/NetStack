#pragma once

#include <stdint.h>

#include "np_layer.h"
#include "TpSocket.h"

#include "InterfaceControl_SFBusSlave.h"
#include "InterfaceControl_SFBusMaster.h"

#define NUM_OF_INTERFACES	5

extern NpLayer *interfaces[NUM_OF_INTERFACES];
extern TpSocket tpSocket1;
extern InterfaceControl_SFBusMaster interfaceControlMaster1;
void Interfaces_Init(uint16_t address);

