/** 
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */
#include "pc_gateway.h"

#include "asf.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

static struct uip_udp_conn *directBroadcastConn;
static struct timer twiPollTimer;

static void initTwiDriver(void);
static inline status_code_t readPCReg(twi_package_t *packet, uint16_t addr, void *buffer, unsigned int len);
static inline status_code_t writePCReg(twi_package_t *packet, uint16_t addr, void *buffer, unsigned int len);


/////////// TODO: implement listener connection

void initPoolControlerGateway(void)
{
    initTwiDriver();
    timer_set(&twiPollTimer, TWI_TIME_BETWEEN_POLLS_LONG_MS);

    uip_ip4addr_t broadcastAddr;
    uip_ip4addr_t subnetMask;

    //If connection is already open, first close this connection
    if(directBroadcastConn != NULL)
    {
        uip_udp_remove(directBroadcastConn);
        directBroadcastConn = NULL;
    }

    uip_ipaddr_copy(&broadcastAddr, &uip_hostaddr);
    uip_ipaddr_copy(&subnetMask, &uip_netmask);

    //Calculate direct broadcast addr..
    // ip addr                  196.168.1.15
    // subnetmask               255.255.0.0
    // direct broadcast addr    196.168.255.255
    broadcastAddr[0] |= (subnetMask[0] ^ 0xFFFF);
    broadcastAddr[1] |= (subnetMask[1] ^ 0xFFFF);

    directBroadcastConn = uip_udp_new(&broadcastAddr, 0);
    if(directBroadcastConn != NULL)
    {
      uip_udp_bind(directBroadcastConn, htons(PC_NETWORK_PORT));
    }
}

void poolControllerGatewayAppCall(void)
{
    twi_package_t packet;
    uint8_t regBuffer[2];
    static uint8_t commErrors = 0;

    packet.chip = TWI_ADDR_MASTER_POOL_CONTROLLER;
    packet.no_wait = false;
    packet.addr_length = TWI_LEN_OF_REG_ADDR;

    if (uip_poll())
    {
        if (uip_udp_conn == directBroadcastConn)
        {
            if (timer_expired(&twiPollTimer))
            {
                timer_restart(&twiPollTimer);

                if (STATUS_OK == readPCReg(&packet, REG_ADDR_STATUS, regBuffer, REG_LEN_STATUS))
                {
                    if (regBuffer[0] & REG_STATUS_NEW_DATA_TO_SEND)
                    {
                        if (STATUS_OK == readPCReg(&packet, REG_ADDR_DATA_LEN_TX, regBuffer, REG_LEN_DATA_LEN_TX))
                        {
                            uint16_t dataLenToSend = (uint16_t)regBuffer[0]|(regBuffer[1] << 8);
                            readPCReg(&packet, REG_ADDR_TX_BUFFER, uip_appdata, dataLenToSend);
                            regBuffer[0] = REG_EVENT_TX_DATA_SENT;
                            writePCReg(&packet, REG_ADDR_EVENTS, regBuffer, REG_LEN_EVENTS);

                            uip_udp_send(dataLenToSend);
                            timer_set(&twiPollTimer, TWI_TIME_BETWEEN_POLLS_LONG_MS);
                        }
                    }

                    commErrors = 0;
                 }
                else if (commErrors > 30)
                {
                    timer_set(&twiPollTimer, TWI_TIME_BETWEEN_POLLS_ERROR_MS);
                }
                else
                {
                    commErrors++;
                }
            }
        }
    }
    else if (uip_newdata())
    {
        if (STATUS_OK == readPCReg(&packet, REG_ADDR_STATUS, regBuffer, REG_LEN_STATUS))
        {
            if (regBuffer[0] & REG_STATUS_RX_BUFFER_FREE)
            {
                writePCReg(&packet, REG_ADDR_RX_BUFFER, uip_appdata, (uip_datalen() > REG_LEN_RX_BUFFER) ? REG_LEN_RX_BUFFER : uip_datalen());
                regBuffer[0] = (uint8_t)(uip_datalen() >> 8);
                regBuffer[1] = (uint8_t)uip_datalen();
                if (STATUS_OK == writePCReg(&packet, REG_ADDR_DATA_LEN_RX, regBuffer, REG_LEN_DATA_LEN_RX))
                {
                    regBuffer[0] = REG_EVENT_NEW_DATA_IN_RX_BUFFER;
                    writePCReg(&packet, REG_ADDR_EVENTS, regBuffer, REG_LEN_EVENTS);
                    timer_set(&twiPollTimer, TWI_TIME_BETWEEN_POLLS_SHORT_MS);  // Look for an answer
                }
            }
        }
    }
}


static void initTwiDriver(void)
{
    twi_options_t twiModuleOptions;
    twiModuleOptions.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED_HZ);
    twiModuleOptions.chip = TWI_ADDR_MASTER_GATEWAY;
    twiModuleOptions.speed = TWI_SPEED_HZ;

    PORTC.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
    PORTC.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;

    sysclk_enable_peripheral_clock(&TWIC);
    twi_master_init(&TWIC, &twiModuleOptions);
    twi_master_enable(&TWIC);
}


static inline status_code_t readPCReg(twi_package_t *packet, uint16_t addr, void *buffer, unsigned int len)
{
    packet->addr[0] = addr;
    packet->addr[1] = addr >> 8;
    packet->buffer = (void *)buffer;
    packet->length = len;
    return twi_master_read(&TWIC, packet);
}

static inline status_code_t writePCReg(twi_package_t *packet, uint16_t addr, void *buffer, unsigned int len)
{
    packet->addr[0] = addr;
    packet->addr[1] = addr >> 8;
    packet->buffer = buffer;
    packet->length = len;
    return twi_master_write(&TWIC, packet);
}
