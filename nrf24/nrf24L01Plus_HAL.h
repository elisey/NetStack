#pragma once

#include <stdint.h>

void nordic_HAL_Init();
uint8_t nordic_HAL_SpiTransfer(uint8_t data);
void nordic_HAL_ChipSelect();
void nordic_HAL_ChipDeselect();

void nordic_HAL_ChipEnableLow();
void nordic_HAL_ChipEnableHigh();

uint8_t nordic_HAL_GetIrqPinState();

extern void nordic_HAL_OnIrqLow();
