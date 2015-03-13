#pragma once

//Frames Settings
#define netConfigNUM_OF_FRAMES				(40)
#define netConfigMAX_FRAME_SIZE				(143)


//SfBus MAC settings
#define SFBus_MAX_PACKET_SIZE				(netConfigMAX_FRAME_SIZE)
#define SFBus_MIN_PACKET_SIZE				(4)
#define SFBus_FRAME_HEAD_SIZE				(4)
#define SFBus_MAX_PAYLOAD_SIZE				(SFBus_MAX_PACKET_SIZE - SFBus_FRAME_HEAD_SIZE)

#define SFBus_RESEND_NUM					(3u)
#define SFBus_WAIT_ACK_TIMEOUT				(2u)

#define SFBus_RX_RAW_QUEUE_SIZE				(10)
#define SFBus_RX_MAC_QUEUE_SIZE				(10)
#define SFBus_RX_ACK_QUEUE_SIZE				(3)

#define SFBus_RX_MAC_TASK_PRIORITY			(2)

//Nrf24 MAC settings
#define nrf24_WAIT_TRANSMIT_STATUS_TIMEOUT	(5)

#define nrf24_NUM_OF_HARDWARE_RETRANSMIT	(2)
#define nrf24_HARDWARE_RETRANSMIT_DELAY		(500)

#define nrf24_CHANNEL_MHZ					(2400)		// 2400 - 2525 MHz
#define nrf24_BITRATE_KBPS					(2000)			// 1000, 2000 Kbps
#define nrf24_RX_MAC_QUEUE_SIZE				(10)
#define nrf24_RX_MAC_TASK_PRIORITY			(2)

//NP Settings
#define np_BROADCAST_ADDRESS				(0)
#define np_TOP_REDIRECTION_ADDRESS			(0xFFFF)

#define np_FRAME_HEAD_SIZE					(7)
#define np_USE_OWN_PACKET_ASSEMBLY			(1)
#define np_ROUTE_OTHER_PACKETS				(1)

#define np_RX_TASK_PRIORITY					(1)

#define np_MAX_TTL							(31)

//NCMP Settings
#define ncmp_RX_QUEUE_SIZE					(10)
#define ncmp_TASK_PRIORITY					(2)

//NCMP Master settings
#define ncmp_NUM_OF_PING_TRY				(3)
#define ncmp_WAIT_FOR_PONG_TIMEOUT			(10)
#define ncmp_PING_PERIOD					(20)

//NCMP Slave settings
#define ncmp_IM_SLAVE_PACKET_PERIOD			(15)
#define ncmp_WAIT_FOR_PING_PERIOD			(50)

//Router table settings
#define rt_ROUTER_TABLE_SIZE				(20)

//Tp socket settings
#define tp_FRAME_HEAD_LENGTH				(4)
#define tp_MAX_TP_PAYLOAD_SIZE				(SFBus_MAX_PACKET_SIZE - SFBus_FRAME_HEAD_SIZE - np_FRAME_HEAD_SIZE - tp_FRAME_HEAD_LENGTH)

#define tp_MAX_NUM_OF_SOCKETS				(10)
#define tp_NUM_OF_RESEND_TRYES				(8)
#define tp_ACK_WAIT_TIMEOUT					(50)
#define tp_RESEND_TIMEOUT_IF_FAIL			(50)


