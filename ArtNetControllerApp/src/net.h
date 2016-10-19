#ifndef NET_H_
#define NET_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "memory.h"
#include "uip.h"

typedef enum
{
    NET_EVENT_START_DHCP            = 0x01u,
    NET_EVENT_STOP_DHCP             = 0x02u,
    NET_EVENT_LINK_UP               = 0x04u,
    NET_EVENT_LINK_DOWN             = 0x08u,
}tNetworkEvent;

typedef enum
{
    NET_STATE_DHCP_IS_ON            = 0x01u,
    NET_STATE_DHCP_IS_ACTIVE        = 0x02u,
    NET_STATE_DHCP_CONFIG_RECEIVED  = 0x04u,
    NET_STATE_LINK_IS_OK            = 0x08u,
}tNetworkStatus;

typedef struct
{
	uip_ip4addr_t ipAddress;
	uip_ip4addr_t subnetmask;
	uip_ip4addr_t defaultRouter;
}tNetworkParams;

typedef struct uip_eth_addr tEthernetAddress;

void initNetwork(void);
void processNetwork(void);

void setNetworkEvent(tNetworkEvent event);
tNetworkStatus getNetworkStatus(void);

bool setNetworkParams(const tNetworkParams *params);
void getNetworkParams(tNetworkParams *params);
bool setEthernetAddress(const tEthernetAddress *eth);
void getEthernetAddress(tEthernetAddress *eth);

#endif /* NETWORK_H_ */
