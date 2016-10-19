/*
 * Simple common network interface that all network drivers should implement.
 */

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "uip.h"

/*Initialize the network*/
void network_init(void);

/*Initialize the network with a mac addr*/
void network_init_mac(const uint8_t* macaddr);

/*Read from the network, returns number of read bytes*/
uint16_t network_read(void);

/*Send using the network*/
void network_send(void);

/*Sets the MAC address of the device*/
void network_set_MAC(const struct uip_eth_addr * const mac);

/*Gets the MAC address of the device*/
void network_get_MAC(struct uip_eth_addr * const mac);

#endif /* __NETWORK_H__ */
