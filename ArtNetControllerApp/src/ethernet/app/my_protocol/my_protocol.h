#ifndef MY_PROTOCOL_H_
#define MY_PROTOCOL_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////
// protocol specific definitions

#define MY_PACKET_HEADER						0xFFu
#define MY_PACKET_VERSION						0x40u

// command definitions
#define MY_PROT_CMD_JUMP_BOOTLOADER			'K'
#define MY_PROT_CMD_UPDATE_APP				'L'
#define MY_PROT_CMD_JUMP_APPLICATION		'M'
#define MY_PROT_CMD_SET_DMX_ADDR		  'O'
#define MY_PROT_CMD_BLINK_IF_DMX_ADDR	   'o'
#define MY_PROT_CMD_SET_VALUE               'S'
#define MY_PROT_CMD_GET_VALUE               'G'


// network addresses
#define MY_PROT_NWA_MASTER								0xFEFEFE00ul				// Address of service pc
#define MY_PROT_NWA_BROADCAST							0xFEFEFA00ul				// broadcast address for all DMX modules

// positions of struct PROGRAM UPDATE
#define MY_PROT_PROG_PACKET_POS_ID			0u
#define MY_PROT_PROG_PACKET_POS_RESERVED_ID	1u
#define MY_PROT_PROG_PACKET_POS_PAGE_ADDR	3u
#define MY_PROT_PROG_PACKET_POS_DATA		5u

// Positions of struct get value
#define MY_PROT_SETGET_STRUCT_POS_ID          0u
#define MY_PROT_SETGET_STRUCT_POS_VERSION     2u
#define MY_PROT_SETGET_STRUCT_POS_LEN         3u
#define MY_PROT_SETGET_STRUCT_POS_DATA       5u

// Header and raw data definitions
#define MY_PACKET_POS_HEADER			0u
#define MY_PACKET_POS_VERSION			1u
#define MY_PACKET_POS_DEST_ADDR			2u
#define MY_PACKET_POS_SRC_ADDR			6u
#define MY_PACKET_POS_LEN_OF_DATA		10u
#define MY_PACKET_POS_COMMAND			12u
#define MY_PACKET_POS_DATA				13u

#define MY_PACKET_LEN_OF_LEN_OF_DATA	2u
#define MY_PACKET_MAX_LEN_OF_DATA		261u // size of struct 'Application Update'
#define MY_PACKET_LEN_OF_HEADER			13u
#define MY_PACKET_LEN_OF_SRC_ADDR		4u
#define MY_PACKET_LEN_OF_DEST_ADDR		4u
#define MY_PACKET_LEN_OF_HEADER_AND_TRAILOR	( 1 + 1 + MY_PACKET_LEN_OF_DEST_ADDR + MY_PACKET_LEN_OF_SRC_ADDR + 2 + 1  + 1)
#define MY_PACKET_MAX_LEN_OF_FRAME      (MY_PACKET_MAX_LEN_OF_DATA + MY_PACKET_LEN_OF_HEADER_AND_TRAILOR)

#define MY_PROT_DATA_ACK_VALUE			0xFDu
#define MY_PROT_LEN_OF_ACK				1u

#define MY_PROT_NAK_VALUE				0xFCu
#define MY_PROT_LEN_OF_NAK				2u
#define MY_PROT_NAK_REASON_UNKNOWN_COMMAND		1u
#define MY_PROT_NAK_REASON_WRONG_DATA_STRUCT	2u
#define MY_PROT_NAK_REASON_APPLICATION_FAULTY	10u
/////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// struct ACK
#define DATA_ACK									0xFD
#define LEN_OF_ACK									1
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// struct NAK
#define DATA_NAK									0xFC
#define LEN_OF_NAK									2

#define NAK_REASON_NO_ERROR						0
#define NAK_REASON_UNKNOWN_COMMAND				1
#define NAK_REASON_WRONG_DATA_STRUCT			2
#define NAK_REASON_DATA_NOT_DEFINED				3
#define NAK_REASON_STORING_ERROR				4
#define NAK_REASON_COMMAND_NOT_ALLOWED			5
#define NAK_REASON_WRONG_STRUCT_VERSION			6
#define NAK_REASON_WRONG_STRUCT_LEN				7
#define NAK_REASON_WRONG_TYPE					8
#define NAK_REASON_WRONG_RUN_MODE				9
#define NAK_REASON_APPLICATION_FAULTY			10
#define NAK_REASON_UNKNOWN_COMMAND_BOOTLOADER	11
#define NAK_REASON_SYNC_NOT_ACTIVE				12
#define NAK_REASON_UNKNOWN_VALUE_IN_REQUEST		13
////////////////////////////////////////////////////////////////////////////////



////////// === UDP ====
#define MY_PROT_UDP_PORT                    51304u
#define MY_PROT_MAX_ALLOWED_CONNECTIONS     1
#define MY_PROT_CONNECTION_TIMEOUT_MS       10000u


//////// ==== My Protocol ====
typedef struct 
{
	uint8_t sign;
	uint8_t version;
	uint32_t destAddr;
	uint32_t srcAddr;
	uint16_t lenOfData;
	uint8_t command;
    uint8_t data[MY_PACKET_MAX_LEN_OF_DATA];
}tMyProtPacket;



// My Packet container
typedef struct
{
	uint32_t srcAddr;
	uint32_t destAddr;
	uint16_t lenOfData;
	uint8_t command;
	uint8_t data[MY_PACKET_MAX_LEN_OF_DATA];
}tMyPacketContainer;


typedef struct  
{
    uint32_t myAddress;
    uint8_t myAppId;
}tMyProtocolConfig;

typedef struct  
{
    uint32_t myAddress;
    uint8_t myAppId;
    
}tMyProtocol;


void initMyProtocol(void);
void myProtocolAppCall(void);

#if defined PORT_APP_MAPPER
#define MY_PROTOCOL_APP_CALL_MAP {myProtocolAppCall, MY_PROT_UDP_PORT, 0},
#else
#define MY_PROTOCOL_APP_CALL_MAP
#define UIP_UDP_APPCALL myProtocolAppCall
typedef int8_t uip_udp_appstate_t;
#endif

#endif /* MY_PROTOCOL_H_ */