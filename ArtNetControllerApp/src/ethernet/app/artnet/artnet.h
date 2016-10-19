/*
 * artnet.h
 *
 * Created: 19.04.2015 20:41:19
 *  Author: Christian
 */ 


#ifndef ARTNET_H_
#define ARTNET_H_

#include <stdint.h>
#include <stdbool.h>

#include "artnet_defs.h"
#include "apps-conf.h"
#include "uip-conf.h"
#include "timer.h"

#define MY_ESTA_ID_HI   0xEE
#define MY_ESTA_ID_LO   0xEE
#define MY_OEM_VALUE_HI 0xEE
#define MY_OEM_VALUE_LO 0xEE



/** \name Art net protol id str definitions 
 * @{ */
#define ART_NET_PROTOCOL_ID_STR		"Art-Net"
#define LEN_OF_PROTOCOL_ID_STR      8
/** @} */

/** \name TalkToMe
 * @{ */
#define TALK_TO_ME_UNUSED_BITS_MSK          0xF1
#define TALK_TO_ME_DIAG_MSG_ARE_UNICAST     0x08
#define TALK_TO_ME_SEND_DIAG_MSG            0x04
#define TALK_TO_ME_SEND_ART_POLL_AUTOMATIC  0x02
/** @} */

/** \name Status1
 * @{ */
#define STAT1_INDICATOR_MSK				0xC0
#define STAT1_INDICATOR_UNKNOWN_VAL		0x00
#define STAT1_INDICATOR_LOCATE_MODE_VAL 0x40
#define STAT1_INDICATOR_MUTE_MODE_VAL	0x80
#define STAT1_INDICATOR_NORMAL_MODE_VAL	0xC0

#define STAT1_PORT_ADDR_PROG_AUTH_MSK	0x30
#define STAT1_PORT_ADDR_UNKNOWN_VAL		0x00
#define STAT1_PORT_ADDR_BY_FRONT_PANEL_VAL 0x10
#define STAT1_PORT_ADDR_BY_NETWORK_VAL	0x20                         
#define STAT1_PORT_ADDR_NOT_USED_VAL	0x30   

#define STAT1_BOOT_FROM_ROM			0x04
#define STAT1_RDM_SUPPORTED	        0x02
#define STAT1_UEBA_PRESENT			0x01 
/** @} */                    


#define LEN_OF_SHORT_NAME       18
#define LEN_OF_LONG_NAME        64
#define LEN_OF_NODE_REPORT      64

#define MAX_NUMBER_OF_PORTS 4

/** \name PortType definitions
 * @{ */
#define PORT_TYPE_IS_OUTPUT     0x80
#define PORT_TYPE_IS_INPUT      0x40

#define PORT_TYPE_DEF_MSK           0x3F
#define PORT_TYPE_DEF_IS_DMX512_VAL 0x00
#define PORT_TYPE_DEF_IS_MIDI_VAL   0x01
#define PORT_TYPE_DEF_IS_AVAB_VAL   0x02
#define PORT_TYPE_DEF_IS_COLOTRAN_CMX_VAL   0x03
#define PORT_TYPE_DEF_IS_ADB_62_5_VAL   0x04
#define PORT_TYPE_DEF_IS_ART_NET_VAL    0x05
/** @} */


/** \name GoodInput definitions
 * @{ */
#define GOOD_INP_DATA_RECEIVED                  0x80
#define GOOD_INP_CHAN_INC_DMX512_TEST_PACKETS   0x40
#define GOOD_INP_CHAN_INC_DMX512_SIPS           0x20
#define GOOD_INP_CHAN_INC_DMX512_TEXT_PACKETS   0x10
#define GOOD_INP_INPUT_IS_DISABLED              0x08
#define GOOD_INP_RECEIVE_ERRORS_DETECTED        0x04
#define GOOD_INP_UNUSED_BITS_MSK                0x03
/** @} */

/** \name GoodOutput definitions
 * @{ */
#define GOOD_OUT_DATA_TRANSMITTED               0x80
#define GOOD_OUT_CHAN_INC_DMX512_TEST_PACKETS   0x40
#define GOOD_OUT_CHAN_INC_DMX512_SIPS           0x20
#define GOOD_OUT_CHAN_INC_DMX512_TEXT_PACKETS   0x10
#define GOOD_OUT_IS_MERGING_ART_NET_DATA        0x08
#define GOOD_OUT_DMX_OUTPUT_SHORT_DETECTED      0x04
#define GOOD_OUT_MERGE_MODE_IS_LTP              0x02
#define GOOD_OUT_UNUSED_BITS_MSK                0x01
/** @} */

/** \name NetSwitch
 * @{ */
#define NET_SWITCH_PROGRAMM_ADDR        0x80
#define NET_SWITCH_ADDR_MSK			    0x7F
#define NET_SWITCH_NO_CHANGE_VAL        0x7F
#define NET_SWITCH_RESET_TO_DEFAULT_VAL 0x00
#define NET_SWITCH_DEFAULT_VAL          0x00
/** @} */

/** \name SubSwitch
 * @{ */
#define SUB_SWITCH_PROGRAMM_ADDR    0x80
#define SUB_SWITCH_ADDR_MSK			0x0F
#define SUB_SWITCH_NO_CHANGE_VAL    0x7F
#define SUB_SWITCH_RESET_TO_DEFAULT_VAL 0x00
#define SUB_SWITCH_DEFAULT_VAL      0x00
/** @} */

/** \name SwIn 
 * @{ */   
#define SW_IN_PROGRAMM_PORT_ADDR    0x80
#define SW_IN_PORT_ADDR_MSK         0x0F
#define SW_IN_NO_CHANGE_VAL         0x7F
#define SW_IN_RESET_TO_DEFAULT_VAL  0x00
#define SW_IN_PORT1_DEFAULT_VAL     0x00
#define SW_IN_PORT2_DEFAULT_VAL     0x00
#define SW_IN_PORT3_DEFAULT_VAL     0x00
#define SW_IN_PORT4_DEFAULT_VAL     0x00
/** @} */

/** \name SwOut 
 *  @{ */   
#define SW_OUT_PROGRAMM_PORT_ADDR   0x80
#define SW_OUT_PORT_ADDR_MSK        0x0F
#define SW_OUT_NO_CHANGE_VAL        0x7F
#define SW_OUT_RESET_TO_DEFAULT_VAL 0x00
#define SW_OUT_PORT1_DEFAULT_VAL     0x00
#define SW_OUT_PORT2_DEFAULT_VAL     0x00
#define SW_OUT_PORT3_DEFAULT_VAL     0x00
#define SW_OUT_PORT4_DEFAULT_VAL     0x00
/** @} */


/** \name SwVideo
 * @{ */
#define SW_VIDEO_SHOWING_LOCAL_DATA_VAL 0x00
#define SW_VIDEO_SHOWING_ETH_DATA_VAL   0x01
/** @} */

/** \name SwMacro
 * @{ */
#define SW_MACRO_8_ACTIVE   0x80
#define SW_MACRO_7_ACTIVE   0x40
#define SW_MACRO_6_ACTIVE   0x20
#define SW_MACRO_5_ACTIVE   0x10
#define SW_MACRO_4_ACTIVE   0x08
#define SW_MACRO_3_ACTIVE   0x04
#define SW_MACRO_2_ACTIVE   0x02
#define SW_MACRO_1_ACTIVE   0x01
/** @} */

/** \name SwRemote
 * @{ */
#define SW_REMOTE_8_ACTIVE   0x80
#define SW_REMOTE_7_ACTIVE   0x40
#define SW_REMOTE_6_ACTIVE   0x20
#define SW_REMOTE_5_ACTIVE   0x10
#define SW_REMOTE_4_ACTIVE   0x08
#define SW_REMOTE_3_ACTIVE   0x04
#define SW_REMOTE_2_ACTIVE   0x02
#define SW_REMOTE_1_ACTIVE   0x01
/** @} */

/** \name MAC
 * @{ */
#define MAC_CANNOT_BE_DISPLAYED 0
/** @} */

/** \name BindIndex
 * @{ */
#define BIND_INDEX_NO_BINDING  0
/** @} */

/** \name Status2
 * @{ */
#define STAT2_UNUSED_BITS_MSK           0xF0
#define STAT2_IP_IS_SUPPLIED_FROM_DHCP  0x08   ///< If this bit is cleared, The IP is static
#define STAT2_SUPPORTS_DHCP             0x04   
#define STAT2_SUPPORTS_15BIT_ART_ADDRESS 0x02   ///< If this bit is cleared, a 8 bit address is supported
#define STAT2_SUPPORTS_BROWSER_CONFIG    0x01
/** @} */

/** \name Command
 * @{ */
#define COMMAND_ENABLE_PROGRAMMING      0x80u
#define COMMAND_ENABLE_DHCP             0x40u
#define COMMAND_UNUSED_BITS_MSK         0x30u
#define COMMAND_RESET_NETWORK_PARAM     0x08u
#define COMMAND_PROGRAMM_IP_ADDR        0x04u
#define COMMAND_PROGRAMM_SUBNET_MASK    0x02u
#define COMMAND_PROGRAMM_PORT           0x01u
/** @} */

/** \name STATUS
 * @{ */
#define STAT_DHCP_ENABLED       0x40
#define STAT_UNUSED_BITS_MSK    0xBF
/** @} */

/** \name Frames
 * @{ */
#define FRAMES_MAX_VAL  29
#define FRAMES_MIN_VAL  0
/** @} */

/** \name Type
 * @{ */
#define TYPE_FILM_24FPS_VAL     0
#define TYPE_EBU_25FPS_VAL      1
#define TYPE_DF_29_97FPS_VAL    2
#define TYPE_SMPTE_30FPS_VAL    3
/** @} */

/** \name OemCode
 * @{ */
#define OEM_CODE_HI_ALL_MANUFACTURE  0xFF
#define OEM_CODE_LO_ALL_MANUFACTURE  0xFF
/** @} */

/** \name Key
 * @{ */
#define KEY_ALL_MANUFACTURE_ASCII   0
#define KEY_ALL_MANUFACTURE_MACRO   1
#define KEY_ALL_MANUFACTURE_SOFT    2
#define KEY_ALL_MANUFACTURE_SHOW    3
/** @} */

/** \name Sequence 
 * @{ */
#define SEQUENCE_DISABLED   0x00
#define SEQUENCE_MIN_VAL    0x01
#define SEQUENCE_MAX_VAL    0xFF
/** @} */

/** \name Input
 * @{ */
#define INPUT_UNUSED_BITS_MSK   0xFE
#define INPUT_DISABLE           0x01
/** @} */

/** \name Firmware Type
 * @{ */
#define FIRM_TYPE_FIRM_FIRST_VAL    0x00
#define FIRM_TYPE_FIRM_CONT_VAL    0x01
#define FIRM_TYPE_FIRM_LAST_VAL     0x02
#define FIRM_TYPE_UBEA_FIRST_VAL    0x03
#define FIRM_TYPE_UBEA_CONT_VAL    0x04
#define FIRM_TYPE_UBEA_LAST_VAL     0x05
/** @} */

/** \name Firmware Reply Type
 * @{ */
#define FIRM_REPLY_TYPE_FIRM_BLOCK_GOOD 0x00
#define FIRM_REPLY_TYPE_FIRM_ALL_GOOD   0x01
#define FIRM_REPLY_TYPE_FIRM_FAIL       0xFF
/** @} */


#define NUMBER_OF_NODE_REPORT_MSG   16

#define LEN_OF_PAYLOAD  512
#define FILLER_VAL  0
#define SPARE_VAL   0


#define LEN_OF_IPV4_NWA   4

typedef struct S_ArtNetPacketHeader
{
    uint8_t ID[8];                    // protocol ID = "Art-Net"
    uint16_t OpCode;                  // == OpPoll
    uint8_t ProtVerHi;                // 0
    uint8_t ProtVerLo;                // protocol version, set to ProtocolVersion
}T_ArtNetPacketHeader;


typedef struct S_ArtNetBindAddress
{
    uint8_t ip[LEN_OF_IPV4_NWA];
    uint8_t index;
}T_ArtNetBindAddress;


typedef struct S_ArtPortAddress 
{
    uint8_t NetSwitch;  // --> Net
    uint8_t SubSwitch;  // --> Sub-net

    uint8_t SwIn[MAX_NUMBER_OF_PORTS];
    uint8_t SwOut[MAX_NUMBER_OF_PORTS];
}T_ArtPortAddress;


typedef struct S_ArtPortStatus
{
    uint8_t NumberOfPorts;
    uint8_t PortTypes[MAX_NUMBER_OF_PORTS];
    uint8_t GoodInput[MAX_NUMBER_OF_PORTS];
    uint8_t GoodOutput[MAX_NUMBER_OF_PORTS];
}T_ArtPortStatus;


typedef struct S_ArtNodeStatus
{
	uint8_t Status;
	uint8_t Status1;
	uint8_t Status2;
    
    uint16_t networkPort;

    uint8_t TalkToMe;

    uint8_t SwMacro;
    uint8_t SwRemote;
    uint8_t SwVideo;

    uint16_t StatusCode;
    uint32_t PollReplyCount;    
}T_ArtNodeStatus;



typedef struct S_ArtNodeInfo
{
// 	char *ID;               //replaced by a const
// 	uint8_t Style;          //replaced by a const
// 	uint8_t VersInfoH;
// 	uint8_t VersInfoL;
// 	uint8_t OemHi;          //replaced by a const
// 	uint8_t OemLo;          //replaced by a const
// 	uint8_t EstaManHi;      //replaced by a const
// 	uint8_t EstaManLo;      //replaced by a const
	uint8_t UebaVersion;    //a bios verion
	uint8_t ShortName[LEN_OF_SHORT_NAME];
	uint8_t LongName[LEN_OF_LONG_NAME];
}T_ArtNodeInfo;


#define ART_NET_MAX_NUMBER_OF_UNICAST_CONNS         5u
#define ART_NET_MAX_NUMBER_OF_CONN_APP_STATES    (ART_NET_MAX_NUMBER_OF_UNICAST_CONNS + 2u)

typedef struct S_ArtNodeConnToController
{
    struct uip_udp_conn *connection;
    struct timer timeout;
}T_ArtNetControllerDescriptor;


typedef struct
{
    bool cablePluggedIn             :1;
    bool startDmxOutputOnSync       :1;
    bool receivedIpFromDhcp         :1;
}tStructArtAppEvent;

#define ART_NET_EVENT_CABLE_PLUGGED_IN          0x01
#define ART_NET_EVENT_START_DMX_OUTPUT_ON_SYNC  0x02
#define ART_NET_EVENT_IP_SET_FROM_DHCP          0x04

typedef struct
{
    bool isArtNetInterfaceActive            :1;
    bool syncModeIsActive                   :1;
    bool ipIsReceivedFromDhcp               :1;
    bool gotPermisionToOutput               :1;
    bool isDmxReceivedFromMultipleSources   :1;
    bool masterControllerIsTimedOut         :1;
}tStructArtAppStatus;

typedef struct
{
    bool sendArtPollReplyAsDirBroadcast :1;
    bool sendArtFwReplyAsUnicast        :1;
    bool sendArtDiagDataAsDirBroadcast  :1;
    bool sendArtDiagDataAsUnicast       :1;
}tStructArtCommEvent;

#define ART_NET_COMM_EVENT_SEND_ART_POLL_REPLY      0x01
#define ART_NET_COMM_EVENT_SEND_FW_REPLAY           0x02
#define ART_NET_COMM_EVENT_SEND_DIAG_DATA           0x04

#define ART_NET_COMM_STATUS_CONNECTED               0x01
#define ART_NET_COMM_STATUS_TIMED_OUT               0x02


#define TOUT_UNICAST_CONN_TO_CONTROLLER_MS          (CLOCK_SECOND * 10)
#define TOUT_BETWEEN_MASTER_CONTROLLER_SWITCHES     (CLOCK_SECOND * 11)
#define TOUT_UNTIL_SWITCH_TO_ASYNC_MODE             (CLOCK_SECOND * 4)
#define TIME_BETWEEN_BROADCASTING_ART_POLL_REPLY    (CLOCK_SECOND * 3)


typedef struct S_ArtNetAppState
{
    //uint16_t networkPort;

    //tStructArtAppEvent events;
    
    
//     tStructArtAppStatus status;
//     tStructArtCommEvent commEvents;

//     struct uip_udp_conn *directBroadcastConn;
//     struct uip_udp_conn *controllerListenerConn;
// 
//     T_ArtNetControllerDescriptor *controllerWhoNeedsDiagData;
//     T_ArtNetControllerDescriptor *actualMasterController;
//     T_ArtNetControllerDescriptor controllerList[MAX_NUMBER_OF_UNICAST_CONNS];

    struct timer syncModeTimer;
    struct timer masterControllerSwitchTimer;
    struct timer multipleMasterErrorTimer;
}T_ArtNetAppState;




void initArtNetNode(void);
void artNetAppCall(void);
void setArtNetEvent(uint8_t event);


#if defined PORT_APP_MAPPER
#define ARTNET_APP_CALL_MAP {artNetAppCall, DefaultPort, 0},
#else
#define ARTNET_APP_CALL_MAP
#define UIP_UDP_APPCALL artNetAppCall
typedef int8_t uip_udp_appstate_t;      // Connection appstate handling is the same without port_app_mapper
#ifndef PORT_APP_MAPPER_APPSTATE_UNALLOCATED    
#   define PORT_APP_MAPPER_APPSTATE_UNALLOCATED -1
#endif
#endif

#endif /* ARTNET_H_ */
