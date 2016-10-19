#include "uip.h"
#include "uip_arp.h"
#include "global-conf.h"
#include "artnet.h"

#include "infoBlock.h"
#include "indicator.h"
#include "memory.h"
#include "net.h"
#include "lighting.h"

#include "asf.h"
#include <assert.h>
#include <string.h>

#if (ART_NET_MAX_NUMBER_OF_CONN_APP_STATES > INT8_MAX)
#error A maximum of INT8_MAX appstates can be used
#endif

#define ART_NET_TOUT_BETWEEN_MASTER_CONTROLLER_SWITCHES     10000

typedef enum
{
    CONN_BROADCAST,
    CONN_DIRECT_BROADCAST,
    CONN_UNICAST,
}tConnectionType;

typedef struct
{
    uint8_t commStatus;
    uint8_t commEvent;
    struct timer timeoutTimer;
}tConnectionAppState;


#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define PSTR
#define sprintf_P    sprintf
#define strlcpy_P    strlcpy
#define memcmp_P    memcmp
#endif 
const char ART_PROTOCOL_ID_STR[] PROGMEM = "Art-Net";
const char ART_NODE_REPORT_MSG_STR0[] PROGMEM = "Booted in debug mode (Only used in development)";
const char ART_NODE_REPORT_MSG_STR1[] PROGMEM = "Power On Tests successful";
const char ART_NODE_REPORT_MSG_STR2[] PROGMEM = "Hardware tests failed at Power On";
const char ART_NODE_REPORT_MSG_STR3[] PROGMEM = "Last UDP from Node failed due to truncated length, Most likely caused by a collision.";
const char ART_NODE_REPORT_MSG_STR4[] PROGMEM = "Unable to identify last UDP transmission. Check OpCode and packet length.";
const char ART_NODE_REPORT_MSG_STR5[] PROGMEM = "Unable to open Udp Socket in last transmission attempt";
const char ART_NODE_REPORT_MSG_STR6[] PROGMEM = "Confirms that Short Name programming via ArtAddress, was successful.";
const char ART_NODE_REPORT_MSG_STR7[] PROGMEM = "Confirms that Long Name programming via ArtAddress, was successful.";
const char ART_NODE_REPORT_MSG_STR8[] PROGMEM = "DMX512 receive errors detected.";
const char ART_NODE_REPORT_MSG_STR9[] PROGMEM = "Ran out of internal DMX transmit buffers.";
const char ART_NODE_REPORT_MSG_STR10[] PROGMEM = "Ran out of internal DMX Rx buffers.";
const char ART_NODE_REPORT_MSG_STR11[] PROGMEM = "Rx Universe switches conflict.";
const char ART_NODE_REPORT_MSG_STR12[] PROGMEM = "Product configuration does not match firmware.";
const char ART_NODE_REPORT_MSG_STR13[] PROGMEM = "DMX output short detected. See GoodOutput field.";
const char ART_NODE_REPORT_MSG_STR14[] PROGMEM = "Last attempt to upload new firmware failed.";
const char ART_NODE_REPORT_MSG_STR15[] PROGMEM = "User changed switch settings when address locked by remote programming. User changes ignored.";
const char* const ART_NODE_REPORT_MSG_STR[] PROGMEM = { ART_NODE_REPORT_MSG_STR0, ART_NODE_REPORT_MSG_STR1, ART_NODE_REPORT_MSG_STR2,
                                                        ART_NODE_REPORT_MSG_STR3, ART_NODE_REPORT_MSG_STR4, ART_NODE_REPORT_MSG_STR5,
                                                        ART_NODE_REPORT_MSG_STR6, ART_NODE_REPORT_MSG_STR7, ART_NODE_REPORT_MSG_STR8,
                                                        ART_NODE_REPORT_MSG_STR9, ART_NODE_REPORT_MSG_STR10, ART_NODE_REPORT_MSG_STR11,
                                                        ART_NODE_REPORT_MSG_STR12, ART_NODE_REPORT_MSG_STR13, ART_NODE_REPORT_MSG_STR14,
                                                        ART_NODE_REPORT_MSG_STR15};




static T_ArtNetBindAddress bindAddress;
static T_ArtPortAddress portAddress;
static T_ArtPortStatus portStatus;
static T_ArtNodeStatus nodeStatus;
static T_ArtNodeInfo nodeInfo;

static T_ArtNetAppState app;

static uint8_t artNetEvents;
struct uip_udp_conn *listener;

static struct uip_udp_conn *directedBroadcastConnection;
static tConnectionAppState *directedBroadcastAppState;
static tConnectionAppState appStateList[ART_NET_MAX_NUMBER_OF_CONN_APP_STATES];

// Extended UIP functions
static void deleteConnection(struct uip_udp_conn *conn);
static int8_t allocateNewAppState(void);
//static bool changeAppPort(uint16_t newPort);

static void initDirectedBroadcastConnection(void);
static void getDirectBroadcastAddress(u16_t *ipAddrBuffer);
//static void getSrcIpAddressOfUdpIpPacketHeader(u16_t *ipAddrBuffer);
static tConnectionType getConnTypeOfUdpIpPacketHeader(void);

static bool checkPacketHeader(const T_ArtNetPacketHeader *packetHeader);

static void processIncomingPacket(const void *packet, tConnectionAppState *appState);
static void processPacketArtPoll(const T_ArtPoll *packet, tConnectionAppState *appState);
static void processPacketIpProg(const T_ArtIpProg *packet, tConnectionAppState *appState);
static void processPacketArtFirmwareMaster(const T_ArtFirmwareMaster *packet, tConnectionAppState *appState);
static void processPacketArtAddress(const T_ArtAddress *packet, tConnectionAppState *appState);
static void processPacketArtDmx(const T_ArtDmx *packet, tConnectionAppState *appState);

static size_t buildPacketArtPollReply(void *buffer);
static size_t buildPacketArtIpProgReply(void *buffer);
static size_t buildPacketArtDiagData(void *buffer, uint8_t priority);
static size_t buildPacketArtFirmwareReply(void *buffer, uint8_t firmReplyType);
static size_t buildNodeReportMsg(void *buffer);
static size_t buildNodeDiagDataString(void *buffer);

static void getDefaultPortAddressValues(T_ArtPortAddress *portAddress);
static tEepromStatus writePortAddressToEeprom(const T_ArtPortAddress *portAddress);
static tEepromStatus readPortAddressFromEeprom(T_ArtPortAddress *portAddress);

static void getDefaultNodeStatusValues(T_ArtNodeStatus *nodeStatus);
static tEepromStatus writeNodeStatusToEeprom(const T_ArtNodeStatus *nodeStatus);
static tEepromStatus readNodeStatusFromEeprom(T_ArtNodeStatus *nodeStatus);

/**
 * Initializes all data of the Art-Net node, sets up a permanent direct Broadcast connection (TX only)
 * and a listener connection.
 */
void initArtNetNode(void)
{
    portStatus.NumberOfPorts = 1;
    portStatus.GoodInput[0] = GOOD_INP_INPUT_IS_DISABLED;
    portStatus.GoodInput[1] = GOOD_INP_INPUT_IS_DISABLED;
    portStatus.GoodInput[2] = GOOD_INP_INPUT_IS_DISABLED;
    portStatus.GoodInput[3] = GOOD_INP_INPUT_IS_DISABLED;
    portStatus.PortTypes[0] = PORT_TYPE_DEF_IS_DMX512_VAL|PORT_TYPE_IS_OUTPUT;
    
    if (EEPROM_ALL_OK != readPortAddressFromEeprom(&portAddress))
    {
        getDefaultPortAddressValues(&portAddress);
    }

    if (EEPROM_ALL_OK != readNodeStatusFromEeprom(&nodeStatus))
    {
        getDefaultNodeStatusValues(&nodeStatus);
    }
    nodeStatus.Status2 = STAT2_SUPPORTS_15BIT_ART_ADDRESS|STAT2_SUPPORTS_DHCP;
    nodeStatus.StatusCode = RcPowerOk;

    initDirectedBroadcastConnection();
    
    uip_udp_listen(htons(nodeStatus.networkPort));
    port_app_mapper_change_udp_port(artNetAppCall, nodeStatus.networkPort, 0);

    timer_set(&app.syncModeTimer, TOUT_UNTIL_SWITCH_TO_ASYNC_MODE);
    timer_set(&app.multipleMasterErrorTimer, TOUT_BETWEEN_MASTER_CONTROLLER_SWITCHES);
    timer_set(&app.masterControllerSwitchTimer, TOUT_BETWEEN_MASTER_CONTROLLER_SWITCHES);
    
    if (nodeStatus.Status & STAT_DHCP_ENABLED)
    {
        setNetworkEvent(NET_EVENT_START_DHCP);
    }
}


/**
 * Function is called from Udp/Ip Stack periodically or if new data is received.
 */
void artNetAppCall(void)
{
    if (uip_udp_conn->appstate <= PORT_APP_MAPPER_APPSTATE_UNALLOCATED 
     || uip_udp_conn->appstate > ((sizeof(appStateList) / sizeof(tConnectionAppState)) - 1))
    {
        uip_udp_conn->appstate = allocateNewAppState();
        
        if (uip_udp_conn->appstate == PORT_APP_MAPPER_APPSTATE_UNALLOCATED)
        {
            return;                         // No appstate available --> return
        }
    }
    
    tConnectionAppState *appState = (tConnectionAppState *)&appStateList[uip_udp_conn->appstate];
    
    
    if (uip_poll())
    {
//         if (app.status.syncModeIsActive)
//         {
//             if(timer_expired(&app.syncModeTimer))
//             {
//                 app.status.syncModeIsActive = false;
//             }
//         }
//         if (!app.status.masterControllerIsTimedOut)
//         {
//             if (timer_expired(&app.masterControllerSwitchTimer))
//             {
//                 app.status.masterControllerIsTimedOut = true;
//             }
//         }
//         if (app.status.isDmxReceivedFromMultipleSources)
//         {
//             if (timer_expired(&app.multipleMasterErrorTimer))
//             {
//                 app.status.isDmxReceivedFromMultipleSources = false;
//             }
//         }
        if (artNetEvents & ART_NET_EVENT_IP_SET_FROM_DHCP)
        {
            artNetEvents &= ~ART_NET_EVENT_IP_SET_FROM_DHCP;
            
            nodeStatus.Status2 |= STAT2_IP_IS_SUPPLIED_FROM_DHCP;
            initDirectedBroadcastConnection();      
        }


        if (appState == directedBroadcastAppState)
        {
            if (appState->commEvent & ART_NET_COMM_EVENT_SEND_ART_POLL_REPLY)
            {
                appState->commEvent &= ~ART_NET_COMM_EVENT_SEND_ART_POLL_REPLY;
                uip_send(uip_appdata, buildPacketArtPollReply(uip_appdata));
            }
            else if (appState->commEvent & ART_NET_COMM_EVENT_SEND_DIAG_DATA)
            {
                appState->commEvent &= ~ART_NET_COMM_EVENT_SEND_DIAG_DATA;
                uip_send(uip_appdata, buildPacketArtDiagData(uip_appdata, DpLow));
            }
        }
        else  /* Unicast Connection polls */
        {
            if (appState->commEvent & ART_NET_COMM_EVENT_SEND_DIAG_DATA)
            {
                appState->commEvent &= ~ART_NET_COMM_EVENT_SEND_DIAG_DATA;
                uip_send(uip_appdata, buildPacketArtDiagData(uip_appdata, DpMed));
            }

            /* Kill connections which are timed out */
            if (timer_expired(&appState->timeoutTimer))
            {
                deleteConnection(uip_udp_conn);
            }
        }
    }
    else if (uip_newdata())
    {
        timer_set(&appState->timeoutTimer, ART_NET_TOUT_BETWEEN_MASTER_CONTROLLER_SWITCHES);
        
        if (checkPacketHeader((T_ArtNetPacketHeader *)uip_appdata))
        {
            processIncomingPacket(uip_appdata, appState);
        } 
    }
}

void setArtNetEvent(uint8_t event)
{
    artNetEvents |= event;
}

static void processIncomingPacket(const void *packet, tConnectionAppState *appState)
{
    T_ArtNetPacketHeader *packetHeader = (T_ArtNetPacketHeader *)packet;
    uint16_t opcode = ((htons(packetHeader->OpCode) << 8) | (htons(packetHeader->OpCode) >> 8));
    
    switch (opcode)
    {
        case OpPoll:
        {
            processPacketArtPoll((T_ArtPoll *)packet, appState);
            break;
        }
        case OpSync:
        {
//             if (app.events.startDmxOutputOnSync)
//             {
//                 app.events.startDmxOutputOnSync = false;
//                 stopEditingDmxFrameBuffer();    //Mark new Frame Buffer for output        //TODO
//             }
            timer_restart(&app.syncModeTimer);
 //           app.status.syncModeIsActive = true;
            break;
        }
       case OpDmx:
       {
//           if (app.status.masterControllerIsTimedOut)
//           {
//               uip_ipaddr_copy(&actualMasterControllerIp, &controller->connection->ripaddr);
//
//               timer_restart(&app.masterControllerSwitchTimer);
//               app.status.masterControllerIsTimedOut = false;
//           }
//
//           if (uip_ipaddr_cmp(&actualMasterControllerIp, &controller->connection->ripaddr))
//           {
//               timer_restart(&app.masterControllerSwitchTimer);
//           }
//           else if (!app.status.isDmxReceivedFromMultipleSources)
//           {
//               timer_restart(&app.multipleMasterErrorTimer);
//               app.status.isDmxReceivedFromMultipleSources = true;
//           }


//           if (!app.status.isDmxReceivedFromMultipleSources)
           {
               processPacketArtDmx((T_ArtDmx *)packet, appState);
           }
           break;
       }
       case OpAddress:
       {
           processPacketArtAddress((T_ArtAddress *)packet, appState);
           break;
       }
       case OpIpProg:
       {
           processPacketIpProg((T_ArtIpProg *)packet, appState);
           break;
       }
       case OpFirmwareMaster:
       {
           processPacketArtFirmwareMaster((T_ArtFirmwareMaster *)packet, appState);
           break;
       }
       default:
       {
           break;
       }
   }
}

static void processPacketArtPoll(const T_ArtPoll *packet, tConnectionAppState *appState)
{
    if (getConnTypeOfUdpIpPacketHeader() == CONN_UNICAST)
    {
        return;
    }

    if(packet->TalkToMe & TALK_TO_ME_SEND_DIAG_MSG)
    {
        if (packet->TalkToMe & TALK_TO_ME_DIAG_MSG_ARE_UNICAST)
        {
            appState->commEvent |= ART_NET_COMM_EVENT_SEND_DIAG_DATA;
        }
        else
        {
            directedBroadcastAppState->commEvent |= ART_NET_COMM_EVENT_SEND_DIAG_DATA;
        }
    }
    
    directedBroadcastAppState->commEvent |= ART_NET_COMM_EVENT_SEND_ART_POLL_REPLY;

    if ((packet->TalkToMe & TALK_TO_ME_UNUSED_BITS_MSK) != (nodeStatus.TalkToMe & TALK_TO_ME_UNUSED_BITS_MSK))
    {
        nodeStatus.TalkToMe = packet->TalkToMe & TALK_TO_ME_UNUSED_BITS_MSK;
        writeNodeStatusToEeprom(&nodeStatus);
    }
}

static void processPacketIpProg(const T_ArtIpProg *packet, tConnectionAppState *appState)
{
    if (getConnTypeOfUdpIpPacketHeader() != CONN_UNICAST)
    {
        return;
    }    

    // Deactivate ip programming. (DHCP is always on)
//      bool isNodeStatusNeededToSave = false;
//     
//     if (packet->Command & COMMAND_ENABLE_PROGRAMMING)
//     {
//         if ((packet->Command & COMMAND_ENABLE_DHCP)
//                 && (!(nodeStatus.Status & STAT_DHCP_ENABLED)))
//         {
//             nodeStatus.Status |= STAT_DHCP_ENABLED;
//             setNetworkEvent(NET_EVENT_START_DHCP);
//             isNodeStatusNeededToSave = true;
//         }
//         else if ((!(packet->Command & COMMAND_ENABLE_DHCP))
//                     && (nodeStatus.Status & STAT_DHCP_ENABLED))
//         {
//             nodeStatus.Status &= ~STAT_DHCP_ENABLED;
//             setNetworkEvent(NET_EVENT_STOP_DHCP);
//             isNodeStatusNeededToSave = true;
//         }
// 
//         tNetworkParams params;
//         getNetworkParams(&params);
// 
//         if (packet->Command & COMMAND_RESET_NETWORK_PARAM)
//         {
//             if (nodeStatus.Status & STAT_DHCP_ENABLED)
//             {
//                 nodeStatus.Status2 &= ~STAT2_IP_IS_SUPPLIED_FROM_DHCP;
// 
//                 uip_ipaddr(&params.ipAddress, UIP_IPADDR0, UIP_IPADDR1, UIP_IPADDR2, UIP_IPADDR3);
//                 uip_ipaddr(&params.subnetmask, UIP_NETMASK0, UIP_NETMASK1, UIP_NETMASK2, UIP_NETMASK3);
//             }
//         }
//         else
//         {
//             if (nodeStatus.Status & STAT_DHCP_ENABLED)
//             {
//                 if (packet->Command & COMMAND_PROGRAMM_IP_ADDR)
//                 {
//                     nodeStatus.Status2 &= ~STAT2_IP_IS_SUPPLIED_FROM_DHCP;
//                     uip_ipaddr(&params.ipAddress, packet->ProgIpHi, packet->ProgIp2, packet->ProgIp1, packet->ProgIpLo);
//                 }
// 
//                 if (packet->Command & COMMAND_PROGRAMM_SUBNET_MASK)
//                 {
//                     uip_ipaddr(&params.subnetmask, packet->ProgSmHi, packet->ProgSm2, packet->ProgSm1, packet->ProgSmLo);
//                 }
//             }
// 
//             if (packet->Command & COMMAND_PROGRAMM_PORT)
//             {
//                 if (changeAppPort((uint16_t)(packet->ProgPortHi << 8) | packet->ProgPortLo))
//                 {
//                     /** \todo implement change app port logic */
//                 }
//             }
//         }
//     }
// 
//     if (isNodeStatusNeededToSave)
//     {
//         writeNodeStatusToEeprom(&nodeStatus);
//     }
//     
//     uip_send(uip_appdata, buildPacketArtIpProgReply(uip_appdata));
}

static void processPacketArtFirmwareMaster(const T_ArtFirmwareMaster *packet, tConnectionAppState *appState)
{
    if (getConnTypeOfUdpIpPacketHeader() != CONN_UNICAST)
    {
        return;
    }


    // No Support for FW Updates
//     if(packet->Type == FIRM_TYPE_FIRM_FIRST_VAL)
//     {
//         uint8_t buffer[4];
// 
//         // Save remote IP address who wants to update me
//         buffer[0] = uip_ipaddr1(&controller->connection->ripaddr);
//         buffer[1] = uip_ipaddr2(&controller->connection->ripaddr);
//         buffer[2] = uip_ipaddr3(&controller->connection->ripaddr);
//         buffer[3] = uip_ipaddr4(&controller->connection->ripaddr);
// 
//         writeEepromStream(EEPROM_ART_FW_UPDATE_IP_ADDR, buffer, EEPROM_ART_FW_UPDATE_IP_LEN, EEPROM_ART_FW_UPDATE_IP_VERSION);
// 
//         //ArtFirmwareReply is sent in Booter
//         reset_do_soft_reset();
//     }
//     else
    {

        uip_send(uip_appdata, buildPacketArtFirmwareReply(uip_appdata, FIRM_REPLY_TYPE_FIRM_FAIL));
    }
}




static void processPacketArtAddress(const T_ArtAddress *packet, tConnectionAppState *appState)
{
    if (getConnTypeOfUdpIpPacketHeader() != CONN_UNICAST)
    {
        return;
    }

    bool isPortAddrNeededToSave = false;

    if (packet->ShortName[0] != 0)
    {
        strlcpy((char *)nodeInfo.ShortName, (char *)packet->ShortName, sizeof(nodeInfo.ShortName));
    }
    if (packet->LongName[0] != 0)
    {
        strlcpy((char *)nodeInfo.LongName, (char *)packet->LongName, sizeof(nodeInfo.LongName));
    }

    if (packet->NetSwitch != NET_SWITCH_NO_CHANGE_VAL)
    {
        if (packet->NetSwitch == NET_SWITCH_RESET_TO_DEFAULT_VAL)
        {
            portAddress.NetSwitch = NET_SWITCH_DEFAULT_VAL;
            isPortAddrNeededToSave = true;
        }
        else if (packet->NetSwitch & NET_SWITCH_PROGRAMM_ADDR)
        {
            portAddress.NetSwitch = packet->NetSwitch & NET_SWITCH_ADDR_MSK;
            isPortAddrNeededToSave = true;
        }
    }

    if (packet->SubSwitch != SUB_SWITCH_NO_CHANGE_VAL)
    {
        if (packet->SubSwitch == SUB_SWITCH_RESET_TO_DEFAULT_VAL)
        {
            portAddress.SubSwitch = SUB_SWITCH_DEFAULT_VAL;
            isPortAddrNeededToSave = true;
        }
        else if (packet->SubSwitch & SUB_SWITCH_PROGRAMM_ADDR)
        {
            portAddress.SubSwitch = packet->SubSwitch & SUB_SWITCH_ADDR_MSK;
            isPortAddrNeededToSave = true;
        }
    }

    for (uint_fast8_t i = 0; i < MAX_NUMBER_OF_PORTS; i++)
    {
        if (packet->SwIn[i] != SW_IN_NO_CHANGE_VAL)
        {
            isPortAddrNeededToSave = true;
            if (packet->SwIn[i] == SW_IN_RESET_TO_DEFAULT_VAL)
            {
                switch (i)
                {
                    case 0:
                    {
                        portAddress.SwIn[i] = SW_IN_PORT1_DEFAULT_VAL;
                        break;
                    }
                    case 1:
                    {
                        portAddress.SwIn[i] = SW_IN_PORT2_DEFAULT_VAL;
                        break;
                    }
                    case 2:
                    {
                        portAddress.SwIn[i] = SW_IN_PORT3_DEFAULT_VAL;
                        break;
                    }
                    case 3:
                    {
                        portAddress.SwIn[i] = SW_IN_PORT4_DEFAULT_VAL;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else if (packet->SwIn[i] & SW_IN_PROGRAMM_PORT_ADDR)
            {
                portAddress.SwIn[i] = packet->SwIn[i] & SW_IN_PORT_ADDR_MSK;
                isPortAddrNeededToSave = true;
            }
        }



        if (packet->SwOut[i] != SW_OUT_NO_CHANGE_VAL)
        {
            isPortAddrNeededToSave = true;
            if (packet->SwOut[i] == SW_OUT_RESET_TO_DEFAULT_VAL)
            {
                switch (i)
                {
                    case 0:
                    {
                        portAddress.SwOut[i] = SW_OUT_PORT1_DEFAULT_VAL;
                        break;
                    }
                    case 1:
                    {
                        portAddress.SwOut[i] = SW_OUT_PORT2_DEFAULT_VAL;
                        break;
                    }
                    case 2:
                    {
                        portAddress.SwOut[i] = SW_OUT_PORT3_DEFAULT_VAL;
                        break;
                    }
                    case 3:
                    {
                        portAddress.SwOut[i] = SW_OUT_PORT4_DEFAULT_VAL;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else if (packet->SwOut[i] & SW_OUT_PROGRAMM_PORT_ADDR)
            {
                portAddress.SwOut[i] = packet->SwOut[i] & SW_OUT_PORT_ADDR_MSK;
                isPortAddrNeededToSave = true;
            }
        }
    }

    switch (packet->Command)
    {
        case AcLedNormal:
        {
            nodeStatus.Status1 &= ~STAT1_INDICATOR_MSK;
            nodeStatus.Status1 |= STAT1_INDICATOR_NORMAL_MODE_VAL;
            break;
        }
        case AcLedMute:
        {
            nodeStatus.Status1 &= ~STAT1_INDICATOR_MSK;
            nodeStatus.Status1 |= STAT1_INDICATOR_MUTE_MODE_VAL;
            break;
        }
        case AcLedLocate:
        {
            nodeStatus.Status1 &= ~STAT1_INDICATOR_MSK;
            nodeStatus.Status1 |= STAT1_INDICATOR_LOCATE_MODE_VAL;
            break;
        }
        case AcResetRxFlags:
        {
            break;
        }
        case AcCancelMerge: //Not supported
        case AcNone:
        default:
        {
            break;
        }
    }

    if (isPortAddrNeededToSave)
    {
        writePortAddressToEeprom(&portAddress);
    }

    uip_send(uip_appdata, buildPacketArtPollReply(uip_appdata));
}

static void processPacketArtDmx(const T_ArtDmx *packet, tConnectionAppState *appState)
{
    uint8_t netSwitch = packet->Net & NET_SWITCH_ADDR_MSK;
    uint8_t subSwitch = (packet->SubUni >> 4) & SUB_SWITCH_ADDR_MSK;
    uint8_t swOut = packet->SubUni & SW_OUT_PORT_ADDR_MSK;

    if ((netSwitch == portAddress.NetSwitch)
            && (subSwitch == portAddress.SubSwitch))
    {
        if (swOut == portAddress.SwOut[0])
        {
            writeFrameBuffer(DMX_UNIVERSE1, (uint8_t*)packet->Data, htons(packet->Length));
        }
        if (swOut == portAddress.SwOut[1])
        {
            writeFrameBuffer(DMX_UNIVERSE2, (uint8_t*)packet->Data, htons(packet->Length));
        }
		if (swOut == portAddress.SwOut[3])
        {
            writeFrameBuffer(DMX_UNIVERSE3, (uint8_t*)packet->Data, htons(packet->Length));
        }
		if (swOut == portAddress.SwOut[4])
        {
            writeFrameBuffer(DMX_UNIVERSE4, (uint8_t*)packet->Data, htons(packet->Length));
        }
    }
}

/**
 *  Builds output packet buffer for ArtPollReply
 *
 *  @param *buffer  Location of the output buffer
 *
 *  @returns The byte length of the build packet
 */
static size_t buildPacketArtPollReply(void *buffer)
{
    assert(buffer != NULL);

    T_ArtPollReply *packet = (T_ArtPollReply *)buffer;

    strlcpy_P((char *)packet->ID, ART_PROTOCOL_ID_STR, sizeof(packet->ID));
    packet->OpCode = ((HTONS(OpPollReply) << 8) | (HTONS(OpPollReply) >> 8));
    packet->BoxAddr.IP[0] = uip_ipaddr1(&uip_hostaddr);
    packet->BoxAddr.IP[1] = uip_ipaddr2(&uip_hostaddr);
    packet->BoxAddr.IP[2] = uip_ipaddr3(&uip_hostaddr);
    packet->BoxAddr.IP[3] = uip_ipaddr4(&uip_hostaddr);
    packet->BoxAddr.Port = ((htons(DefaultPort) << 8) | (htons(DefaultPort) >> 8));
    packet->VersionInfoHi = readMajorFwVersion();
    packet->VersionInfoLo = readMinorFwVersion();
    packet->NetSwitch = portAddress.NetSwitch;
    packet->SubSwitch = portAddress.SubSwitch;
    packet->OemHi = MY_OEM_VALUE_HI;
    packet->OemLo = MY_OEM_VALUE_LO;
    packet->UbeaVersion = nodeInfo.UebaVersion;
    packet->Status = nodeStatus.Status1;
    packet->EstaManLo = MY_ESTA_ID_LO;
    packet->EstaManHi = MY_ESTA_ID_HI;
    strlcpy((char *)packet->ShortName, (char *)nodeInfo.ShortName, sizeof(packet->ShortName));
    strlcpy((char *)packet->LongName, (char *)nodeInfo.LongName, sizeof(packet->LongName));
    buildNodeReportMsg((void *)packet->NodeReport);
    packet->NumPortsHi = 0;
    packet->NumPortsLo = portStatus.NumberOfPorts;
    memcpy(packet->PortTypes, portStatus.PortTypes, sizeof(packet->PortTypes));
    memcpy(packet->GoodInput, portStatus.GoodInput, sizeof(packet->GoodInput));
    memcpy(packet->GoodOutput, portStatus.GoodOutput, sizeof(packet->GoodOutput));
    memcpy(packet->SwIn, portAddress.SwIn, sizeof(packet->SwIn));
    memcpy(packet->SwOut, portAddress.SwOut, sizeof(packet->SwOut));
    packet->SwVideo = nodeStatus.SwVideo;
    packet->SwMacro = nodeStatus.SwMacro;
    packet->SwRemote = nodeStatus.SwRemote;
    packet->Spare1 = SPARE_VAL;
    packet->Spare2 = SPARE_VAL;
    packet->Spare3 = SPARE_VAL;
    packet->Style = StyleNode;
    memcpy(packet->Mac, uip_ethaddr.addr, sizeof(packet->Mac));
    memcpy(packet->BindIp, bindAddress.ip, sizeof(packet->BindIp));
    packet->BindIndex = bindAddress.index;
    packet->Status2 = nodeStatus.Status2;
    memset(packet->Filler, FILLER_VAL, sizeof(packet->Filler));

    nodeStatus.PollReplyCount++;

    return sizeof(T_ArtPollReply);
}


/**
 *  Builds output packet buffer for ArtIpProgReplay
 *
 *  @param *buffer  Location of the output buffer
 *
 *  @returns The byte length of the build packet
 */
static size_t buildPacketArtIpProgReply(void *buffer)
{
    assert(buffer != NULL);

    T_ArtIpProgReply *packet = (T_ArtIpProgReply *)buffer;

    strlcpy_P((char *)packet->ID, ART_PROTOCOL_ID_STR, sizeof(packet->ID));
    packet->OpCode = ((HTONS(OpIpProgReply) << 8) | (HTONS(OpIpProgReply) >> 8));
    packet->ProtVerHi = 0;
    packet->ProtVerLo = ProtocolVersion;
    packet->Filler1 = FILLER_VAL;
    packet->Filler2 = FILLER_VAL;
    packet->Filler3 = FILLER_VAL;
    packet->Filler4 = FILLER_VAL;
    packet->ProgIpHi = uip_ipaddr1(&uip_hostaddr);
    packet->ProgIp2 = uip_ipaddr2(&uip_hostaddr);
    packet->ProgIp1 = uip_ipaddr3(&uip_hostaddr);
    packet->ProgIpLo = uip_ipaddr4(&uip_hostaddr);
    packet->ProgSmHi = uip_ipaddr1(&uip_netmask);
    packet->ProgSm2 = uip_ipaddr2(&uip_netmask);
    packet->ProgSm1 = uip_ipaddr3(&uip_netmask);
    packet->ProgSmLo = uip_ipaddr4(&uip_netmask);
    packet->ProgPortHi = (uint8_t)nodeStatus.networkPort >> 8;
    packet->ProgPortLo = (uint8_t)nodeStatus.networkPort;
    packet->Status = nodeStatus.Status;
    packet->Spare2 = SPARE_VAL;
    packet->Spare3 = SPARE_VAL;
    packet->Spare4 = SPARE_VAL;
    packet->Spare5 = SPARE_VAL;
    packet->Spare6 = SPARE_VAL;
    packet->Spare7 = SPARE_VAL;
    packet->Spare8 = SPARE_VAL;

    return sizeof(T_ArtIpProgReply);
}



static size_t buildPacketArtDiagData(void *buffer, uint8_t priority)
{
    assert(buffer != NULL);
    assert(priority == DpLow || priority == DpMed || priority == DpHigh
        || priority == DpCritical || priority == DpVol);

    T_ArtDiagData *packet = (T_ArtDiagData *)buffer;

    strlcpy_P((char *)packet->ID, ART_PROTOCOL_ID_STR, sizeof(packet->ID));
    packet->OpCode = ((HTONS(OpDiagData) << 8) | (HTONS(OpDiagData) >> 8));
    packet->ProtVerHi = 0;
    packet->ProtVerLo = ProtocolVersion;
    packet->Filler1 = FILLER_VAL;
    packet->Priority = priority;
    packet->Filler2 = FILLER_VAL;
    packet->Filler3 = FILLER_VAL;
    packet->Length = htons(buildNodeDiagDataString((void*)packet->Data));

    return sizeof(T_ArtDiagData);
}

static size_t buildPacketArtFirmwareReply(void *buffer, uint8_t firmReplyType)
{
    assert(buffer != NULL);
    assert(firmReplyType == FIRM_REPLY_TYPE_FIRM_ALL_GOOD
            || firmReplyType == FIRM_REPLY_TYPE_FIRM_BLOCK_GOOD
            || firmReplyType == FIRM_REPLY_TYPE_FIRM_FAIL);

    T_ArtFirmwareReply *packet = (T_ArtFirmwareReply *)buffer;

    strlcpy_P((char *)packet->ID, ART_PROTOCOL_ID_STR, sizeof(packet->ID));
    packet->OpCode = ((HTONS(OpFirmwareReply) << 8) | (HTONS(OpFirmwareReply) >> 8));
    packet->ProtVerHi = 0;
    packet->ProtVerLo = ProtocolVersion;
    packet->Filler1 = FILLER_VAL;
    packet->Filler2 = FILLER_VAL;
    packet->Type = firmReplyType;
    memset(packet->Spare, SPARE_VAL, sizeof(packet->Spare));

    return sizeof(T_ArtFirmwareReply);
}

static size_t buildNodeReportMsg(void *buffer)
{
    sprintf_P(buffer, PSTR("#%04x[%d]"), nodeStatus.StatusCode, nodeStatus.PollReplyCount);
    size_t pos = strlen(buffer);
    strlcpy_P((char *)buffer + pos, pgm_read_ptr(&ART_NODE_REPORT_MSG_STR[nodeStatus.StatusCode]), LEN_OF_NODE_REPORT - pos);

    return strlen(buffer);
}

static size_t buildNodeDiagDataString(void *buffer)
{
    //No Diag data for the moment
    char yes[] = "YES";
    char no[] = "NO";
    uint16_t pos = 0;

    sprintf_P((char *)buffer + pos, PSTR("Multiple Master Error: %s\r\n"),
            (true) ? yes : no);
    pos = strlen(buffer);

    sprintf_P((char *)buffer + pos, PSTR("Firmware Update Support: %s\r\n"), no);
    pos = strlen(buffer);

    return pos;
}

static void initDirectedBroadcastConnection(void)
{
    if (directedBroadcastConnection != NULL)
    {
        deleteConnection(directedBroadcastConnection);
    }
    
    uip_ip4addr_t directedBroadcastAddr;
    getDirectBroadcastAddress(directedBroadcastAddr);
    
    directedBroadcastConnection = uip_udp_new(&directedBroadcastAddr, HTONS(DefaultPort));
    if(directedBroadcastConnection != NULL)
    {
        uip_udp_bind(directedBroadcastConnection, htons(nodeStatus.networkPort));
        directedBroadcastConnection->appstate = allocateNewAppState();
        
        if (directedBroadcastConnection->appstate != PORT_APP_MAPPER_APPSTATE_UNALLOCATED)
        {
            directedBroadcastAppState = &appStateList[directedBroadcastConnection->appstate];
            directedBroadcastAppState->commEvent |= ART_NET_COMM_EVENT_SEND_ART_POLL_REPLY;
        }
    }
    else
    {
        uip_udp_remove(directedBroadcastConnection);
    }
}

static void deleteConnection(struct uip_udp_conn *conn)
{
    assert(conn != NULL);
    
    appStateList[conn->appstate].commStatus &= ~ART_NET_COMM_STATUS_CONNECTED;    // Delete appstate
    uip_udp_conn->appstate = PORT_APP_MAPPER_APPSTATE_UNALLOCATED;      // Remove appstate from connection
    uip_udp_remove(uip_udp_conn);                                       // Remove connection
}

static int8_t allocateNewAppState(void)
{
    for (int8_t i = 0; i < ART_NET_MAX_NUMBER_OF_CONN_APP_STATES; i++)
    {
        if (!(appStateList[i].commStatus & ART_NET_COMM_STATUS_CONNECTED))
        {
            appStateList[i].commStatus = ART_NET_COMM_STATUS_CONNECTED;
            appStateList[i].commEvent = 0;
            return i;
        }
    }
    
    return PORT_APP_MAPPER_APPSTATE_UNALLOCATED;
}


static void getDirectBroadcastAddress(u16_t *ipAddrBuffer)
{
    uip_ip4addr_t subnetMask;
    
    uip_ipaddr_copy(ipAddrBuffer, &uip_hostaddr);
    uip_ipaddr_copy(&subnetMask, &uip_netmask);
     
    //Calculate direct broadcast addr..
    // ip addr                  196.168.1.15
    // subnetmask               255.255.0.0
    // direct broadcast addr    196.168.255.255
    ipAddrBuffer[0] |= (subnetMask[0] ^ 0xFFFF);
    ipAddrBuffer[1] |= (subnetMask[1] ^ 0xFFFF);
}

/* static void getSrcIpAddressOfUdpIpPacketHeader(u16_t *ipAddrBuffer)
{
    const struct uip_udpip_hdr *packetHeader = (struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN];

    uip_ipaddr_copy(ipAddrBuffer, &packetHeader->srcipaddr);
} */

static tConnectionType getConnTypeOfUdpIpPacketHeader(void)
{
    uip_ip4addr_t directBroadcastAddr;
    uip_ip4addr_t broadcastAddr;
    tConnectionType connType;
    
    const struct uip_udpip_hdr *packetHeader = (struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN];

    //Calculate direct broadcast addr..
    // ip addr                  196.168.1.15
    // subnetmask               255.255.0.0
    // direct broadcast addr    196.168.255.255
    directBroadcastAddr[0] = uip_hostaddr[0] | (uip_netmask[0] ^ 0xFFFF);
    directBroadcastAddr[1] = uip_hostaddr[1] | (uip_netmask[1] ^ 0xFFFF);

    uip_ipaddr(&broadcastAddr, 255, 255, 255, 255);

    if (uip_ipaddr_cmp(&packetHeader->destipaddr, &directBroadcastAddr))
    {
        connType = CONN_DIRECT_BROADCAST;
    }
    else if (uip_ipaddr_cmp(&packetHeader->destipaddr, &broadcastAddr))
    {
        connType = CONN_BROADCAST;
    }
    else
    {
        connType = CONN_UNICAST;
    }

    return connType;
}

static bool checkPacketHeader(const T_ArtNetPacketHeader *packetHeader)
{
    return (memcmp_P(packetHeader->ID, ART_PROTOCOL_ID_STR, LEN_OF_PROTOCOL_ID_STR) == 0)
          && (packetHeader->ProtVerHi >= 0
                && packetHeader->ProtVerLo >= ProtocolVersion);
}

static void getDefaultPortAddressValues(T_ArtPortAddress *portAddress)
{
    portAddress->NetSwitch = NET_SWITCH_DEFAULT_VAL;
    portAddress->SubSwitch = SUB_SWITCH_DEFAULT_VAL;
    portAddress->SwIn[0] = SW_IN_PORT1_DEFAULT_VAL;
    portAddress->SwIn[1] = SW_IN_PORT2_DEFAULT_VAL;
    portAddress->SwIn[2] = SW_IN_PORT3_DEFAULT_VAL;
    portAddress->SwIn[3] = SW_IN_PORT4_DEFAULT_VAL;
    portAddress->SwIn[0] = SW_IN_PORT1_DEFAULT_VAL;
    portAddress->SwIn[1] = SW_IN_PORT2_DEFAULT_VAL;
    portAddress->SwIn[2] = SW_IN_PORT3_DEFAULT_VAL;
    portAddress->SwIn[3] = SW_IN_PORT4_DEFAULT_VAL;
}

static tEepromStatus writePortAddressToEeprom(const T_ArtPortAddress *portAddress)
{
    uint8_t buffer[EEPROM_ART_ADDRESS_STRUCT_LEN];

    buffer[0] = portAddress->NetSwitch;
    buffer[1] = portAddress->SubSwitch;
    buffer[2] = portAddress->SwIn[0];
    buffer[3] = portAddress->SwIn[1];
    buffer[4] = portAddress->SwIn[2];
    buffer[5] = portAddress->SwIn[3];
    buffer[6] = portAddress->SwIn[0];
    buffer[7] = portAddress->SwIn[1];
    buffer[8] = portAddress->SwIn[2];
    buffer[9] = portAddress->SwIn[3];

    return writeEepromStream(   EEPROM_ART_ADDRESS_STRUCT_ADDR,
                                buffer,
                                EEPROM_ART_ADDRESS_STRUCT_LEN,
                                EEPROM_ART_ADDRESS_STRUCT_VERSION);
}

static tEepromStatus readPortAddressFromEeprom(T_ArtPortAddress *portAddress)
{
    uint8_t buffer[EEPROM_ART_ADDRESS_STRUCT_LEN];
    tEepromStatus status = readEepromStream(EEPROM_ART_ADDRESS_STRUCT_ADDR,
                                                buffer,
                                                EEPROM_ART_ADDRESS_STRUCT_LEN,
                                                EEPROM_ART_ADDRESS_STRUCT_VERSION);

    if (EEPROM_ALL_OK == status)
    {
        portAddress->NetSwitch = buffer[0];
        portAddress->SubSwitch = buffer[1];
        portAddress->SwIn[0] = buffer[2];
        portAddress->SwIn[1] = buffer[3];
        portAddress->SwIn[2] = buffer[4];
        portAddress->SwIn[3] = buffer[5];
        portAddress->SwIn[0] = buffer[6];
        portAddress->SwIn[1] = buffer[7];
        portAddress->SwIn[2] = buffer[8];
        portAddress->SwIn[3] = buffer[9];
    }

    return status;
}

static void getDefaultNodeStatusValues(T_ArtNodeStatus *nodeStatus)
{
    nodeStatus->Status = STAT_DHCP_ENABLED;
    nodeStatus->Status1 = STAT1_INDICATOR_NORMAL_MODE_VAL | STAT1_PORT_ADDR_UNKNOWN_VAL;
    nodeStatus->TalkToMe = 0;
    nodeStatus->PollReplyCount = 0;
    nodeStatus->networkPort = DefaultPort;
}

static tEepromStatus writeNodeStatusToEeprom(const T_ArtNodeStatus *nodeStatus)
{
    uint8_t buffer[EEPROM_ART_NODE_STATUS_STRUCT_LEN];

    buffer[0] = nodeStatus->Status;
    buffer[1] = nodeStatus->Status1;
    buffer[2] = nodeStatus->TalkToMe;
    buffer[3] = (uint8_t)(nodeStatus->PollReplyCount >> 24);
    buffer[4] = (uint8_t)(nodeStatus->PollReplyCount >> 16);
    buffer[5] = (uint8_t)(nodeStatus->PollReplyCount >> 8);
    buffer[6] = (uint8_t)nodeStatus->PollReplyCount;
    buffer[7] = (uint8_t)(nodeStatus->networkPort >> 8);
    buffer[8] = (uint8_t)nodeStatus->networkPort;

    return writeEepromStream(   EEPROM_ART_NODE_STATUS_STRUCT_ADDR,
                                buffer,
                                EEPROM_ART_NODE_STATUS_STRUCT_LEN,
                                EEPROM_ART_NODE_STATUS_STRUCT_VERSION);
}

static tEepromStatus readNodeStatusFromEeprom(T_ArtNodeStatus *nodeStatus)
{
    uint8_t buffer[EEPROM_ART_NODE_STATUS_STRUCT_LEN];
    tEepromStatus status = readEepromStream(EEPROM_ART_NODE_STATUS_STRUCT_ADDR,
                                                buffer,
                                                EEPROM_ART_NODE_STATUS_STRUCT_LEN,
                                                EEPROM_ART_NODE_STATUS_STRUCT_VERSION);

    if (EEPROM_ALL_OK == status)
    {
        nodeStatus->Status = buffer[0];
        nodeStatus->Status1 = buffer[1];
        nodeStatus->TalkToMe = buffer[2];
        nodeStatus->PollReplyCount  = (uint32_t)buffer[3] << 24;
        nodeStatus->PollReplyCount |= (uint32_t)buffer[4] << 16;
        nodeStatus->PollReplyCount |= (uint32_t)buffer[5] << 8;
        nodeStatus->PollReplyCount |= (uint32_t)buffer[6];
        nodeStatus->networkPort = (uint16_t)(buffer[7] << 8) | buffer[8];
    }

    return status;
}


