/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
#include "nrf24L01Plus.h"
#include "nrf24L01Plus_HAL.h"

#define commandACTIVATE					(0x50)
#define commandR_RX_PL_WID				(0x60)
#define commandW_TX_PAYLOAD_NO_ACK		(0xB0)

#define registerFEATURE					(0x1D)
#define registerDYNPD					(0x1C)

// LOW LEVEL NORDIC IO FUNCTION:
static uint8_t nordic_transfer(uint8_t command, uint8_t* data, unsigned short length, bool copy)
{
	int i = 0;

	NORDIC_LOCK_SPI();
	nordic_HAL_ChipSelect();

	uint8_t status = nordic_HAL_SpiTransfer(command);
	if(copy) {
		for( i = 0; i < length; i++) {
			data[i] = nordic_HAL_SpiTransfer(data[i]);
		}
	}
	else {
        for( i = 0; i < length; i++) {
        	nordic_HAL_SpiTransfer(data[i]);
        }
	}

	nordic_HAL_ChipDeselect();
	NORDIC_UNLOCK_SPI();

	return status;
}

#define nordic_exchangeData(command, data, length)	\
			nordic_transfer(command, data, length, true)
#define nordic_outputData(command, data, length)	\
			nordic_transfer(command, data, length, false)

static uint8_t nordic_readRegister(uint8_t reg)
{
	uint8_t data = 0;
	uint8_t opcode = (reg & 0x1F) | 0x00;

	nordic_exchangeData(opcode, &data, 1);
	return data;
}

static void nordic_writeRegister(uint8_t reg, uint8_t data)
{
	uint8_t opcode = (reg & 0x1F) | 0x20;
	nordic_outputData(opcode, &data, 1);
}

static uint8_t nordic_readStatusRegister()
{
	return nordic_exchangeData(0xFF, 0, 0);
}

void nordic_init(
		uint8_t* selfAddress,
		uint8_t* broadcastAddress,
		uint8_t payload,
		unsigned short mhz,
		unsigned short bitrate_kbps,
		unsigned short address_width)
{
	nordic_HAL_Init();
	nordic_toggle_feature();
	nordic_set_features(true, false, false);
	nordic_enable_dynamic_payload_length(true, true, false, false, false, false);

	nordic_flush_rx_fifo();
	nordic_flush_tx_fifo();

	nordic_HAL_ChipEnableLow();
	nordic_power_down();
	nordic_set_intr_signals(false, true, true);
	nordic_clear_all_intr_flags();
	nordic_set_crc(2);

	nordic_set_channel(mhz);
	nordic_set_air_data_rate(bitrate_kbps);
	nordic_set_power_level(3);

	nordic_enable_pipes(true, true, false, false, false, false);
	nordic_set_auto_ack_for_pipes(true, false, false, false, false, false);

	nordic_set_auto_transmit_options(750, 3);

	nordic_set_addr_width(address_width);

	nordic_set_rx_pipe0_addr(selfAddress, address_width);
	nordic_set_rx_pipe1_addr(broadcastAddress, address_width);
	nordic_set_tx_address(selfAddress, address_width);

	nordic_power_up();

	volatile unsigned int i;
	for (i = 0; i < 900000; ++i) {

	}
	NORDIC_DELAY_US(2000);
}

bool nordic_is_air_free()
{
	return (nordic_readRegister(0x09) & (1<<0));
}

bool nordic_intr_signal()
{
	if (nordic_HAL_GetIrqPinState() == 0)	{
		return true;
	}
	return false;
}

bool nordic_is_tx_fifo_full()
{
	return !!(nordic_readStatusRegister() & (1<<0));
}
bool nordic_is_tx_fifo_empty()
{
	return !!(nordic_readRegister(0x17) & (1<<4));
}
void nordic_clear_all_intr_flags()
{
	nordic_writeRegister(7, 0x70);
}
void nordic_queue_tx_fifo(uint8_t *data, unsigned short length)
{
	nordic_outputData(0xA0, data, length);
}

bool nordic_mode1_send_single_packet(uint8_t *data, unsigned short length)
{
    nordic_flush_tx_fifo();
    nordic_queue_tx_fifo(data, length);
    nordic_HAL_ChipEnableHigh();
    int timeout = 0;
    sendingState_t sendingState;
    do {
    	if (++timeout >= 1000)	{
    		break;
    	}
		sendingState = nordic_get_sending_state_and_clear_flag();
	} while (sendingState == sendingState_inProcess );

    nordic_HAL_ChipEnableLow();
    nordic_flush_tx_fifo();

    if (sendingState == sendingState_Ok)	{
    	return true;
    }
    return false;
}

void nordic_mode1_start_send_single_packet(uint8_t *data, unsigned short length)
{
    nordic_flush_tx_fifo();
    nordic_queue_tx_fifo(data, length);
    nordic_HAL_ChipEnableHigh();
}

void nordic_mode1_finish_send_single_packet()
{
    nordic_HAL_ChipEnableLow();
    nordic_flush_tx_fifo();
}

void nordic_standby1_to_tx_mode1()
{
	nordic_writeRegister(0, (nordic_readRegister(0) & ~0x01));	// Set the PRIM_RX to 0
}
void nordic_standby1_to_tx_mode2()
{
	nordic_standby1_to_tx_mode1();
	nordic_HAL_ChipEnableHigh();
}

void nordic_rx_to_Stanby1()
{
	nordic_HAL_ChipEnableLow();
}
void nordic_standby1_to_rx()
{
	nordic_writeRegister(0, (nordic_readRegister(0) | 0x01));	// Set the PRIM_RX to 1
	nordic_HAL_ChipEnableHigh();
}


// TO DO:
void nordic_finishTxMode2_transitionToStandby1(uint8_t* data, unsigned short length)
{
	// Wait until any previous Tx Packets are sent such that Nordic will be in Standby-1 mode.
	while(!nordic_is_tx_fifo_empty());	//TODO проверка и журналирование ошибки таймата

	// From Standby-1 or Standby-2, filling FIFO goes to "Tx Settling" State.
	// Set CE to low while Nordic is in "Tx Setting" state.
	nordic_mode1_send_single_packet(data, length);

	// After packet is sent, Nordic will enter STANDBY-1 since CE is low
	while(nordic_HAL_GetIrqPinState() == 0);

	//while(!nordic_isTxFifoEmpty());
	nordic_clear_packet_sent_flag();
}

void nordic_tx_mode2_to_standby1_through_power_down()
{
	nordic_HAL_ChipEnableLow();
	nordic_power_down();
	nordic_power_up();
}


bool nordic_is_sending()
{

    /* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
    if((nordic_readStatusRegister()&((1 << 4)|(1 << 5))))
    {
        return false; /* false */
    }

    return true; /* true */

}

sendingState_t nordic_get_sending_state_and_clear_flag()
{
	uint8_t status = nordic_readStatusRegister();

	bool packetSent = !!(status & (1 << 5));
	if (packetSent == true)	{
		nordic_clear_packet_sent_flag();
		return sendingState_Ok;
	}
	bool packetFail = !!(status & (1 << 4));
	if (packetFail == true)	{
		nordic_clear_max_retries_flag();
		return sendingState_Fail;
	}
	return sendingState_inProcess;
}

bool nordic_is_packet_sent()
{
	return !!(nordic_readStatusRegister() & (1<<5));
}
void nordic_clear_packet_sent_flag()
{
	nordic_writeRegister(7, (1<<5));
}
bool nordic_is_max_retries_reached()
{
	return !!(nordic_readStatusRegister() & (1<<4));
}
void nordic_clear_max_retries_flag()
{
	nordic_writeRegister(7, (1<<4));
}
void nordic_flush_tx_fifo()
{
	nordic_outputData(0xE1, 0, 0);
}

bool nordic_is_packet_available()
{
    /* Reading status register bit 6 tells us "Data Ready Rx Interrupt"
     * and is manually cleared by clear_packet_available_flag() and doesn't
     * really cover us in the case when we may get multiple rx payloads
     * into the rx fifo. Therefore, reading 0x17 register's bit 0 is
     * more reliable because it states when rx fifo is empty (when 1).
     */
	return !(nordic_readRegister(0x17) & (1<<0));
	//return !!(nordic_readStatusRegister() & (1<<6));
}
void nordic_clear_packet_available_flag()
{
	nordic_writeRegister(7, (1<<6));
}
uint8_t nordic_read_rx_fifo(uint8_t *data, unsigned short length)
{
	return ((nordic_exchangeData(0x61, data, length) & 0x0E) >> 1);
}
void nordic_flush_rx_fifo()
{
	nordic_outputData(0xE2, 0, 0);
}








// CONFIGURATION FUNCTIONS:
void nordic_set_intr_signals(bool rx, bool tx, bool maxTransmissions)
{
	uint8_t configRegister = nordic_readRegister(0);

	configRegister =               rx ? (configRegister & ~(1<<6)) : (configRegister | (1<<6));
	configRegister =               tx ? (configRegister & ~(1<<5)) : (configRegister | (1<<5));
	configRegister = maxTransmissions ? (configRegister & ~(1<<4)) : (configRegister | (1<<4));

	nordic_writeRegister(0, configRegister);
}

uint8_t nordic_get_intr_reg_status(void)
{
	return nordic_readRegister(7);
}

void nordic_set_crc(uint8_t length)
{
	uint8_t configRegister = nordic_readRegister(0);

	configRegister |= (1<<3);						// Enable CRC for now
	if(0 == length) configRegister &= ~(1<<3);		// Disable CRC if length is 0
	else if(1 == length) configRegister &= ~(1<<2);	// 1Byte CRC
	else if(2 == length) configRegister |=  (1<<2);	// 2Byte CRC

	nordic_writeRegister(0, configRegister);
}

void nordic_power_up()
{
	nordic_writeRegister(0, (nordic_readRegister(0) | 0x02));
}
void nordic_power_down()
{
	nordic_writeRegister(0, (nordic_readRegister(0) & ~0x02));
}
void nordic_set_channel(unsigned short MHz)
{
	if(MHz > (2400 + 125)) MHz = (2400+125);
	else if(MHz < 2400)    MHz = 2402;

	MHz -= 2400;
	nordic_writeRegister(5, (uint8_t)MHz);
}
void nordic_set_continous_carrier_transmit(bool enable)
{
	if(enable)
		nordic_writeRegister(6, (nordic_readRegister(6) | 0x80));
	else
		nordic_writeRegister(6, (nordic_readRegister(6) & ~0x80));
}
void nordic_set_air_data_rate(unsigned short kbps)
{
	uint8_t currentRegData = nordic_readRegister(6);

	currentRegData &= ~((1<<5) | (1<<3));	// Set configuration to 1Mbps for now.
	if(250  == kbps)      currentRegData |= (1<<5);
	else if(2000 == kbps) currentRegData |= (1<<3);

	nordic_writeRegister(6, currentRegData);
}
void nordic_set_power_level(uint8_t powerLevel)
{
	uint8_t currentRegData = 0;
	if(powerLevel > 3) powerLevel = 3;

	currentRegData  = nordic_readRegister(6) & ~(0x06);
	currentRegData |= (powerLevel << 1);

	nordic_writeRegister(6, currentRegData);
}







// Nordic Enhanced shockburst Options
void nordic_set_auto_transmit_options(unsigned short txDelayUs, uint8_t retries)
{
	if(txDelayUs < 250)  txDelayUs = 250;
	if(txDelayUs > 4000) txDelayUs = 4000;
	if(retries > 15)     retries = 15;

	uint8_t waitDelay = (txDelayUs / 250) - 1;

	uint8_t controlReg = (waitDelay << 4) | retries;
	nordic_writeRegister(4, controlReg);
}
uint8_t nordic_get_lost_packet_cnt(bool clear)
{
	uint8_t count = nordic_readRegister(8) >> 4;
	if(clear) {
		nordic_writeRegister(5, nordic_readRegister(5));
	}
	return count;
}
uint8_t nordic_get_retransmit_count()
{
	return (nordic_readRegister(8) & 0x0F);
}






// Nordic Address & PIPE Configuration
void nordic_set_payload_for_pipe(uint8_t pipeNumber, uint8_t payload)
{
	const uint8_t pipeAddressBase = 0x11;
	uint8_t pipeAddress = pipeNumber + pipeAddressBase;

	if(pipeAddress > 0x16)
		return; // Invalid Pipe #

	if(payload > 32)
		payload = 32;

	nordic_writeRegister(pipeAddress, payload);
}
void nordic_set_addr_width(unsigned short width)
{
	uint8_t controlReg = 3; // 5-byte

	if(3 == width) controlReg = 1;
	if(4 == width) controlReg = 2;

	nordic_writeRegister(3, controlReg);
}
void nordic_set_tx_address(uint8_t* address, unsigned short length)
{
	nordic_outputData( (0x10 | 0x20), address, length);
}
void nordic_set_rx_pipe0_addr(uint8_t* address, unsigned short length)
{
	nordic_outputData( (0x0A | 0x20), address, length);
}
void nordic_set_rx_pipe1_addr(uint8_t* address, unsigned short length)
{
	nordic_outputData( (0x0B | 0x20), address, length);
}
void nordic_set_rx_pipe2_lsb_addr(uint8_t address)
{
	nordic_writeRegister(0x0C, address);
}
void nordic_set_rx_pipe3_lsb_addr(uint8_t address)
{
	nordic_writeRegister(0x0D, address);
}
void nordic_set_rx_pipe4_lsb_addr(uint8_t address)
{
	nordic_writeRegister(0x0E, address);
}
void nordic_set_rx_pipe5_lsb_addr(uint8_t address)
{
	nordic_writeRegister(0x0F, address);
}

void nordic_set_auto_ack_for_pipes(bool pipe0, bool pipe1, bool pipe2, bool pipe3, bool pipe4, bool pipe5)
{
	uint8_t controlReg = 0;

	if(pipe0) controlReg |= 0x01;
	if(pipe1) controlReg |= 0x02;
	if(pipe2) controlReg |= 0x04;
	if(pipe3) controlReg |= 0x08;
	if(pipe4) controlReg |= 0x10;
	if(pipe5) controlReg |= 0x20;

	nordic_writeRegister(1, controlReg);
}
void nordic_enable_pipes(bool pipe0, bool pipe1, bool pipe2, bool pipe3, bool pipe4, bool pipe5)
{
	uint8_t controlReg = 0;

	if(pipe0) controlReg |= 0x01;
	if(pipe1) controlReg |= 0x02;
	if(pipe2) controlReg |= 0x04;
	if(pipe3) controlReg |= 0x08;
	if(pipe4) controlReg |= 0x10;
	if(pipe5) controlReg |= 0x20;

	nordic_writeRegister(2, controlReg);
}


void nordic_toggle_feature()
{
	uint8_t data = 0x73;
	nordic_outputData(commandACTIVATE, &data, 1);
}

uint8_t nordic_get_rx_payload_width()
{
	uint8_t data = 0;
	uint8_t opcode = commandR_RX_PL_WID;

	nordic_exchangeData(opcode, &data, 1);
	return data;
}

void nordic_queue_tx_fifo_no_ack(uint8_t *data, unsigned short length)
{
	nordic_outputData(commandW_TX_PAYLOAD_NO_ACK, data, length);
}

void nordic_set_features(
		bool enableDinamicPayloadLength,
		bool enablePayloadWithAck,
		bool enableTxPayloadNoAckCommand)
{
	uint8_t data = 0;

	if (enableTxPayloadNoAckCommand)	{
		data |= 0x01;
	}
	if (enablePayloadWithAck)	{
		data |= 0x02;
	}
	if (enableDinamicPayloadLength)	{
		data |= 0x04;
	}
	nordic_writeRegister(registerFEATURE, data);
}

void nordic_enable_dynamic_payload_length(bool pipe0, bool pipe1, bool pipe2, bool pipe3, bool pipe4, bool pipe5)
{
	uint8_t controlReg = 0;

	if(pipe0) controlReg |= 0x01;
	if(pipe1) controlReg |= 0x02;
	if(pipe2) controlReg |= 0x04;
	if(pipe3) controlReg |= 0x08;
	if(pipe4) controlReg |= 0x10;
	if(pipe5) controlReg |= 0x20;

	nordic_writeRegister(registerDYNPD, controlReg);
}
