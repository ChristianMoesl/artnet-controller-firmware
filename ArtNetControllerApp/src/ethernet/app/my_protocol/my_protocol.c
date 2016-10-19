/*
 * my_protocol.c
 *
 * Created: 11.10.2015 21:31:54
 *  Author: chris_000
 */ 
#include "my_protocol.h"
#include "uip.h"
#include "asf.h"
#include "memory.h"

#include <assert.h>
#include <string.h>

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


static tConnectionAppState appStateList[MY_PROT_MAX_ALLOWED_CONNECTIONS];


static bool checkMyPacket(const uint8_t *rawData);
static tConnectionType getConnType(tMyProtPacket *Header);

static int8_t allocateNewAppState(void);
static void deleteConnection(struct uip_udp_conn *conn);

static void processIncomingPacket(const void *packet, uint32_t srcAddr, tConnectionType connType);
static bool saveDataRecords(uint8_t *data, uint16_t dataLen);
static bool getEepromRecordSpecById(const uint8_t *id, uint16_t *dataLen, uint16_t *address);
static uint8_t calculateChecksum(const void *packet, uint16_t dataLen);

static size_t buildPacketAck(void *buffer);
static size_t buildPacketNack(void *buffer, uint8_t nakReason);

void initMyProtocol(void)
{
    uip_udp_listen(HTONS(MY_PROT_UDP_PORT));
}

void myProtocolAppCall(void)
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
        if (timer_expired(&appState->timeoutTimer))
        {
            deleteConnection(uip_udp_conn);
        }
    }
    else if (uip_newdata())
    {
        timer_set(&appState->timeoutTimer, MY_PROT_CONNECTION_TIMEOUT_MS);
        
        if (checkMyPacket((const uint8_t *)uip_appdata))
        {   
            tMyProtPacket *header = (tMyProtPacket *)uip_appdata;
            
            if (ntohl(header->srcAddr) == MY_PROT_NWA_MASTER && ntohl(header->destAddr) == MY_PROT_NWA_BROADCAST)
            {
                switch(header->command)
                {
                    case MY_PROT_CMD_JUMP_BOOTLOADER:
                    {
                        reset_cause_clear_causes(0xFF);
                        reset_do_soft_reset();
                        break;
                    }
                }
            }
        }
    }
}

static bool saveDataRecords(uint8_t *data, uint16_t len)
{
    uint8_t id[2];
    uint8_t version;
    uint16_t dataLen;
    
    uint16_t pos = 0;
    
    while (pos < len)
    {
        id[0] = data[pos + MY_PROT_SETGET_STRUCT_POS_ID + 0];
        id[1] = data[pos + MY_PROT_SETGET_STRUCT_POS_ID + 1];
        version = data[pos + MY_PROT_SETGET_STRUCT_POS_VERSION];
        dataLen = (uint16_t)(data[pos + MY_PROT_SETGET_STRUCT_POS_LEN + 0] << 8) 
                          |  data[pos + MY_PROT_SETGET_STRUCT_POS_LEN + 1];
        
        uint16_t actualDataLen; 
        uint16_t address; 
        
        bool foundEntry = getEepromRecordSpecById(id, &actualDataLen, &address);
        
        if (foundEntry && actualDataLen == dataLen)
        {
            writeEepromStream(address, &data[pos + MY_PROT_SETGET_STRUCT_POS_DATA], dataLen, version);
            pos += MY_PROT_SETGET_STRUCT_POS_DATA + dataLen;
        }
        else
        {
            return false;    
        }
    }
}

static bool getEepromRecordSpecById(const uint8_t *id, uint16_t *dataLen, uint16_t *address)
{
    bool result = true;
    
    if (0 == memcmp(id, EEPROM_TOUCH_DIM_ID, EEPROM_LEN_OF_ID))
    {
        *dataLen = EEPROM_TOUCH_DIM_LEN;
        *address = EEPROM_TOUCH_DIM_ADDR;
    }
    else if (0 == memcmp(id, EEPROM_NET_PARAMS_ID, EEPROM_LEN_OF_ID))
    {
        *dataLen = EEPROM_NET_PARAMS_LEN;
        *address = EEPROM_NET_PARAMS_ADDR;
    }
    else if (0 == memcmp(id, EEPROM_ART_NODE_STATUS_STRUCT_ID, EEPROM_LEN_OF_ID))
    {
        *dataLen = EEPROM_ART_NODE_STATUS_STRUCT_LEN;
        *address = EEPROM_ART_NODE_STATUS_STRUCT_ADDR;
    }   
    else if (0 == memcmp(id, EEPROM_ART_ADDRESS_STRUCT_ID, EEPROM_LEN_OF_ID))
    {
        *dataLen = EEPROM_ART_ADDRESS_STRUCT_LEN;
        *address = EEPROM_ART_ADDRESS_STRUCT_ADDR;
    }   
    else
    {
        result = false;
    }
    
    return result;  
}


static size_t buildPacketAck(void *buffer)
{   
    tMyProtPacket *packet = (tMyProtPacket *)buffer;
    
    uint32_t tempAddress = packet->destAddr;
    packet->destAddr = packet->srcAddr;
    packet->srcAddr = tempAddress;
    
    packet->lenOfData = HTONS(LEN_OF_ACK);
    packet->data[0] = DATA_ACK;
    packet->data[1] = calculateChecksum(packet, LEN_OF_ACK);
    
    return LEN_OF_ACK + MY_PACKET_LEN_OF_HEADER_AND_TRAILOR;
}

static size_t buildPacketNack(void *buffer, uint8_t nakReason)
{
    tMyProtPacket *packet = (tMyProtPacket *)buffer;
    
    uint32_t tempAddress = packet->destAddr;
    packet->destAddr = packet->srcAddr;
    packet->srcAddr = tempAddress;
    
    packet->lenOfData = HTONS(LEN_OF_NAK);
    packet->data[0] = DATA_NAK;
    packet->data[1] = nakReason;
    packet->data[2] = calculateChecksum(packet, LEN_OF_NAK);
    
    return LEN_OF_NAK + MY_PACKET_LEN_OF_HEADER_AND_TRAILOR;
}

static uint8_t calculateChecksum(const void *packet, uint16_t dataLen)
{
    uint8_t *rawData = (uint8_t *)packet;
    
    uint8_t checksum = 0;
    uint_fast16_t i;
    for (i = MY_PACKET_POS_DEST_ADDR; i < (dataLen + MY_PACKET_POS_DATA); i++)
    {
        checksum += rawData[i];
    }
    if (checksum == 0xFF)
    {
        checksum = 0;
    }
    return checksum;
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

static void deleteConnection(struct uip_udp_conn *conn)
{
    assert(conn != NULL);
    
    appStateList[conn->appstate].commStatus &= ~ART_NET_COMM_STATUS_CONNECTED;    // Delete appstate
    uip_udp_conn->appstate = PORT_APP_MAPPER_APPSTATE_UNALLOCATED;      // Remove appstate from connection
    uip_udp_remove(uip_udp_conn);                                       // Remove connection
}

static bool checkMyPacket(const uint8_t *rawData)
{
    tMyProtPacket *header = (tMyProtPacket *)rawData;
    
    if ((header->sign == MY_PACKET_HEADER)
    && (header->version == MY_PACKET_VERSION)
    && (ntohs(header->lenOfData) <= MY_PACKET_MAX_LEN_OF_DATA))
    {
        uint8_t checksum = 0;
        uint_fast16_t i;
        for (i = MY_PACKET_POS_DEST_ADDR; i < (ntohs(header->lenOfData) + MY_PACKET_POS_DATA); i++)
        {
            checksum += rawData[i];
        }
        if (checksum == 0xFF)
        {
            checksum = 0;
        }
        
        // The next byte after the data should be the checksum
        if (checksum == rawData[i])
        {
            return true;
        }            
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


static tConnectionType getConnectionType(tMyProtPacket *deader)
{
}