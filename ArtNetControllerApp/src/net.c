#include "net.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "indicator.h"
#include "global-conf.h"
#include "network.h"
#include "uip_arp.h"
#include "apps-conf.h"
#include "uip.h"

#define MAC_TYPE_GROUP      0x01u
#define MAC_LOCAL_MODIFIED  0x02u

#define uipIOBuffer ((struct uip_eth_hdr *)&uip_buf[0])

static struct timer dhcp_timer, periodic_timer, arp_timer, startup_timer;

static uint8_t netEvent;
static uint8_t netState;

static void onNetworkLinkIsUp(void);
static void onNetworkLinkIsDown(void);
static void onDhcpStartRequested(void);
static void onDhcpStopRequested(void);
static void onNetworkMessageReceived(void);
static void onPeriodicNetworkStackTick(void);

static void serviceUipOutputBuffer(void);
static void serveNetEvents(void);

static void getDefaultNetworkParams(tNetworkParams *params);
static tEepromStatus writeNetworkParamsToEeprom(const tNetworkParams *param);
static tEepromStatus readNetworkParamsFromEeprom(tNetworkParams *param);

static void getDefaultEthernetAddress(tEthernetAddress *eth);

void initNetwork(void)
{
    // Read needed eeprom values
    tNetworkParams params;
    if(EEPROM_ALL_OK != readNetworkParamsFromEeprom(&params))
    {
        getDefaultNetworkParams(&params);
    }
    uip_sethostaddr(&params.ipAddress);
    uip_setnetmask(&params.subnetmask);
    uip_setdraddr(&params.defaultRouter);
    
    tEthernetAddress eth;
    if(EEPROM_ALL_OK != readEepromStream(EEPROM_ETH_ADDRESS_ADDR, eth.addr, EEPROM_ETH_ADDRESS_LEN, EEPROM_ETH_ADDRESS_VERSION))
    {
        getDefaultEthernetAddress(&eth);
    }
    uip_setethaddr(eth);
    
    // Initialize network stack
    clock_init();                   // Initialize ms systick
    network_init_mac(eth.addr);     // Init eth controller
    uip_init();
    uip_arp_init();
    port_app_mapper_init();
    
    // Initialize all network apps
    initMyProtocol();
    initArtNetNode();
    #ifdef DEBUG
 //   initDebugCom();
    #endif

    timer_set(&periodic_timer, CLOCK_SECOND/1000);
    timer_set(&arp_timer, CLOCK_SECOND * 10);
    timer_set(&startup_timer, (clock_time_t)CLOCK_SECOND * 5);
}

static void onNetworkLinkIsUp(void)
{
    netState |= NET_STATE_LINK_IS_OK;
    
    if (netState & NET_STATE_DHCP_IS_ON)
    {
        onDhcpStartRequested();
    }
}

static void onNetworkLinkIsDown(void)
{
    netState &= ~NET_STATE_LINK_IS_OK;
}

static void onDhcpStartRequested(void)
{
    if (netState & NET_STATE_LINK_IS_OK)
    {
        if (!(netState & NET_STATE_DHCP_IS_ACTIVE))
        {
            timer_set(&dhcp_timer, CLOCK_SECOND * 600);

            dhcpc_init(&uip_ethaddr, 6);
            dhcpc_request();
            
            netState |= NET_STATE_DHCP_IS_ACTIVE;
        }
    }
    
    netState |= NET_STATE_DHCP_IS_ON;
}

static void onDhcpStopRequested(void)
{
    netState &= ~(NET_STATE_DHCP_IS_ON | NET_STATE_DHCP_IS_ACTIVE);
}

static void onNetworkMessageReceived(void)
{
    toggleReceiveLed();
    if(uipIOBuffer->type == HTONS(UIP_ETHTYPE_IP))
    {
        uip_arp_ipin();
        uip_input();
        serviceUipOutputBuffer();
    }
    else if(uipIOBuffer->type == HTONS(UIP_ETHTYPE_ARP))
    {
        uip_arp_arpin();
        if(uip_len > 0)
        {
            network_send();
        }
    }
}

static void onPeriodicNetworkStackTick(void)
{
#if UIP_TCP
    for (uint_fast8_t connection = 0; connection < UIP_CONNS; connection++)
    {
        uip_periodic(connection);
        serviceUipOutputBuffer();
    }
#endif
    
#if UIP_UDP
    for (uint_fast8_t connection = 0; connection < UIP_UDP_CONNS; connection++)
    {
        uip_udp_periodic(connection);
        serviceUipOutputBuffer();
    }        
 #endif /* UIP_UDP */
}

static void processDhcpService(void)
{
    if (timer_expired(&dhcp_timer))
    {
        dhcpc_renew();
        timer_reset(&dhcp_timer);
    }
    
//     if ((netState & NET_STATE_DHCP_IS_ON) != 0 && timer_expired(&dhcp_timer))
//     {
//        if (netState & NET_STATE_DHCP_CONFIG_RECEIVED)
//         {
//             dhcpc_renew();
//             timer_reset(&dhcp_timer);
//                // DHCP can't reach the server, set all network params to previous values
//         else 
//         {
//             tNetworkParams params;
//             if(EEPROM_ALL_OK != readNetworkParamsFromEeprom(&params))
//             {
//                 getDefaultNetworkParams(&params);
//             }
//             setNetworkParams(&params);
//             
//             // Quit DHCP mode
//             netState &= ~(NET_STATE_DHCP_IS_ON | NET_STATE_DHCP_IS_ACTIVE);
//         }
//         timer_reset(&dhcp_timer);
//     }
}

void processNetwork(void)
{
    serveNetEvents();
    
	uip_len = network_read();

	if(uip_len > 0) 
    {
		onNetworkMessageReceived();
	}
	else if(timer_expired(&periodic_timer))
    {
		timer_reset(&periodic_timer);
        onPeriodicNetworkStackTick();
	} 
    else if(timer_expired(&arp_timer))
    {
        timer_reset(&arp_timer);
        uip_arp_timer();
    }
    if (netState & NET_STATE_DHCP_IS_ACTIVE)
    {
        processDhcpService();
    }
}

static void serviceUipOutputBuffer(void)
{
    if(uip_len > 0)
    {
        uip_arp_out();
        network_send();
    }
}

void setNetworkEvent(tNetworkEvent event)
{
    netEvent |= event;
}

tNetworkStatus getNetworkStatus(void)
{
    return netState;   
}

void dhcpc_configured(const struct dhcpc_state *s)
{
	tNetworkParams params;

    uip_ipaddr(&params.ipAddress, s->ipaddr[0], s->ipaddr[0] >> 8, s->ipaddr[1], s->ipaddr[1] >> 8);
    uip_ipaddr(&params.subnetmask, s->netmask[0], s->netmask[0] >> 8, s->netmask[1], s->netmask[1] >> 8);
    uip_ipaddr(&params.defaultRouter, s->default_router[0], s->default_router[0] >> 8, s->default_router[1], s->default_router[1] >> 8);

    setNetworkParams(&params);

    netState |= NET_STATE_DHCP_CONFIG_RECEIVED;
    setArtNetEvent(ART_NET_EVENT_IP_SET_FROM_DHCP);


	// Take care off an overflow using a specifc "clock_time_t" type 	
	uint64_t leaseTimeInSec = (uint64_t)ntohs(s->lease_time[0])* UINT16_MAX + ntohs(s->lease_time[1]);
    
    // Renew ip address after 75% of lease time expires
    leaseTimeInSec = leaseTimeInSec / 4 * 3;

	uint64_t leaseTimeForTimer = leaseTimeInSec * CLOCK_SECOND;
	const uint64_t clockTimeMax = (uint64_t)((pow(2, sizeof(clock_time_t) * 8) / 2) - 1);
	
	if(leaseTimeForTimer > clockTimeMax)
	{
		leaseTimeForTimer = clockTimeMax;
	}
	
	timer_set(&dhcp_timer, (clock_time_t)leaseTimeForTimer);
}

void uip_log(char *msg)
{
    while(*msg)
    {
        msg++;
        putchar(*msg);
    }
}

bool setNetworkParams(const tNetworkParams *params)
{
    bool result = false;
    
    if (!(uip_ipaddr_cmp_host(&params->ipAddress))
        || !(uip_ipaddr_cmp(&params->subnetmask, &uip_netmask))
            || !(uip_ipaddr_cmp(&params->defaultRouter, &uip_draddr)))
    {
        uip_sethostaddr(&params->ipAddress);
        uip_setnetmask(&params->subnetmask);
        uip_setdraddr(&params->defaultRouter);
        writeNetworkParamsToEeprom(params);
        
        result = true;
    }
    
    return result;
}

void getNetworkParams(tNetworkParams *params)
{
    uip_gethostaddr(&params->ipAddress);
    uip_getnetmask(&params->subnetmask);
    uip_getdraddr(&params->defaultRouter);
}

bool setEthernetAddress(const tEthernetAddress *eth)
{   
    bool result = false;
    const struct uip_eth_addr ethBroadcast = { .addr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};            
    
    if (memcmp(eth->addr, ethBroadcast.addr, sizeof(eth->addr)))
    {
        uip_setethaddr((*eth));
        writeEepromStream(EEPROM_ETH_ADDRESS_ADDR, eth->addr, EEPROM_ETH_ADDRESS_LEN, EEPROM_ETH_ADDRESS_VERSION);
        
        result = true;
    }
    
    return result;
}

void getEthernetAddress(tEthernetAddress *eth)
{
    memcpy(eth->addr, uip_ethaddr.addr, sizeof(eth->addr));
}

static void serveNetEvents()
{
    if (netEvent)
    {
        if (netEvent & NET_EVENT_LINK_UP)
        {
            onNetworkLinkIsUp();
        }
        if (netEvent & NET_EVENT_LINK_DOWN)
        {
            onNetworkLinkIsDown();
        }
        if (netEvent & NET_EVENT_START_DHCP)
        {
            onDhcpStartRequested();
        }
        if (netEvent & NET_EVENT_STOP_DHCP)
        {
            onDhcpStopRequested();
        }
        netEvent = 0;
    }
}

static void getDefaultNetworkParams(tNetworkParams *params)
{
    uip_ipaddr( &params->ipAddress,
                UIP_IPADDR0,
                UIP_IPADDR1,
                UIP_IPADDR2,
                UIP_IPADDR3);

    uip_ipaddr( &params->subnetmask,
                UIP_NETMASK0,
                UIP_NETMASK1,
                UIP_NETMASK2,
                UIP_NETMASK3);

    uip_ipaddr( &params->defaultRouter,
                UIP_DRIPADDR0,
                UIP_DRIPADDR1,
                UIP_DRIPADDR2,
                UIP_DRIPADDR3);
}


static tEepromStatus writeNetworkParamsToEeprom(const tNetworkParams *param)
{
    uint8_t buffer[EEPROM_NET_PARAMS_LEN];

    buffer[0] = uip_ipaddr1(&param->ipAddress);
    buffer[1] = uip_ipaddr2(&param->ipAddress);
    buffer[2] = uip_ipaddr3(&param->ipAddress);
    buffer[3] = uip_ipaddr4(&param->ipAddress);
    buffer[4] = uip_ipaddr1(&param->subnetmask);
    buffer[5] = uip_ipaddr2(&param->subnetmask);
    buffer[6] = uip_ipaddr3(&param->subnetmask);
    buffer[7] = uip_ipaddr4(&param->subnetmask);
    buffer[8] = uip_ipaddr1(&param->defaultRouter);
    buffer[9] = uip_ipaddr2(&param->defaultRouter);
    buffer[10] = uip_ipaddr3(&param->defaultRouter);
    buffer[11] = uip_ipaddr4(&param->defaultRouter);

    return writeEepromStream(EEPROM_NET_PARAMS_ADDR, buffer, EEPROM_NET_PARAMS_LEN, EEPROM_NET_PARAMS_VERSION);
}

static tEepromStatus readNetworkParamsFromEeprom(tNetworkParams *param)
{
    uint8_t buffer[EEPROM_NET_PARAMS_LEN];
    tEepromStatus status;

    status = readEepromStream(EEPROM_NET_PARAMS_ADDR, buffer, EEPROM_NET_PARAMS_LEN, EEPROM_NET_PARAMS_VERSION);

    if(status == EEPROM_ALL_OK)
    {
        uip_ipaddr( &param->ipAddress,
                    buffer[0],
                    buffer[1],
                    buffer[2],
                    buffer[3]);
        uip_ipaddr( &param->subnetmask,
                    buffer[4],
                    buffer[5],
                    buffer[6],
                    buffer[7]);
        uip_ipaddr( &param->defaultRouter,
                    buffer[8],
                    buffer[9],
                    buffer[10],
                    buffer[11]);
    }

    return status;
}


static void getDefaultEthernetAddress(tEthernetAddress *eth)
{
    struct nvm_device_serial serial;
    nvm_read_device_serial(&serial);
    
    srand(serial.coordx0);
    eth->addr[0] = (uint8_t)rand() & (~MAC_TYPE_GROUP) | MAC_LOCAL_MODIFIED;
    srand(serial.coordx1);
    eth->addr[1] = (uint8_t)rand();
    srand(serial.coordy0);
    eth->addr[2] = (uint8_t)rand();
    srand(serial.coordy1);
    eth->addr[3] = (uint8_t)rand();
    srand(serial.lotnum0);
    eth->addr[4] = (uint8_t)rand();
    srand(serial.lotnum1);
    eth->addr[5] = (uint8_t)rand();
}