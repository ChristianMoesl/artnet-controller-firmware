//Project specific configurations
#ifndef __GLOBAL_CONF_H__
#define __GLOBAL_CONF_H__

#include "asf.h"

//Define frequency
//#define F_CPU 32000000UL

//Default Network parameterers
#define UIP_ETHADDR0    0x00
#define UIP_ETHADDR1    0x01
#define UIP_ETHADDR2    0x02
#define UIP_ETHADDR3    0x03
#define UIP_ETHADDR4    0x04
#define UIP_ETHADDR5    0x05

#define UIP_IPADDR0 192
#define UIP_IPADDR1 168
#define UIP_IPADDR2 1
#define UIP_IPADDR3 210

#define UIP_NETMASK0 255
#define UIP_NETMASK1 255
#define UIP_NETMASK2 255
#define UIP_NETMASK3 0

#define UIP_DRIPADDR0 192
#define UIP_DRIPADDR1 168
#define UIP_DRIPADDR2 1
#define UIP_DRIPADDR3 1


//Include uip.h gives all the uip configurations in uip-conf.h
#include "uip.h"

#endif /*__GLOBAL_CONF_H__*/
