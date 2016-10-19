/** 
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */
#ifndef SRC_ETHERNET_APP_ADAFASDSAD_PC_GATEWAY_H_
#define SRC_ETHERNET_APP_ADAFASDSAD_PC_GATEWAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "uip-conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_NETWORK_PORT 49870

#define TWI_SPEED_HZ                    300000
#define TWI_ADDR_MASTER_POOL_CONTROLLER 0x10
#define TWI_ADDR_MASTER_GATEWAY         0x20
#define TWI_LEN_OF_REG_ADDR             2
#define TWI_TIME_BETWEEN_POLLS_ERROR_MS 1000
#define TWI_TIME_BETWEEN_POLLS_LONG_MS  50
#define TWI_TIME_BETWEEN_POLLS_SHORT_MS 3

#define REG_ADDR_STATUS             0
#define REG_LEN_STATUS              1
#define REG_STATUS_NEW_DATA_TO_SEND  0x01
#define REG_STATUS_RX_BUFFER_FREE    0x02

#define REG_ADDR_EVENTS                 (REG_ADDR_STATUS + REG_LEN_STATUS)
#define REG_LEN_EVENTS                  1
#define REG_EVENT_TX_DATA_SENT          0x01
#define REG_EVENT_NEW_DATA_IN_RX_BUFFER 0x02

#define REG_ADDR_DATA_LEN_TX    (REG_ADDR_EVENTS + REG_LEN_EVENTS)
#define REG_LEN_DATA_LEN_TX     2

#define REG_ADDR_DATA_LEN_RX    (REG_ADDR_DATA_LEN_TX + REG_LEN_DATA_LEN_TX)
#define REG_LEN_DATA_LEN_RX     2

#define REG_ADDR_TX_BUFFER      (REG_ADDR_DATA_LEN_RX + REG_LEN_DATA_LEN_RX)
#define REG_LEN_TX_BUFFER       300

#define REG_ADDR_RX_BUFFER      (REG_ADDR_TX_BUFFER + REG_LEN_TX_BUFFER)
#define REG_LEN_RX_BUFFER       300

typedef struct
{
    
}tStructPCGatewayAppState;


void initPoolControlerGateway(void);
void poolControllerGatewayAppCall(void);

#if defined PORT_APP_MAPPER
#define PC_GATEWAY_APP_CALL_MAP {poolControllerGatewayAppCall, PC_NETWORK_PORT, 0},
#else
#define PC_GATEWAY_APP_CALL_MAP
#define UIP_UDP_APPCALL poolControllerGatewayAppCall
typedef tStructPCGatewayAppState uip_udp_appstate_t;
#endif


#ifdef __cplusplus
}
#endif

#endif /* SRC_ETHERNET_APP_ADAFASDSAD_PC_GATEWAY_H_ */
