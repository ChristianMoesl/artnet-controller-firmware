#ifndef __UIP_CONF_H__
#define __UIP_CONF_H__

#include <inttypes.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * 8 bit datatype
 *
 * This typedef defines the 8-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint8_t u8_t;

/**
 * 16 bit datatype
 *
 * This typedef defines the 16-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint16_t u16_t;

/**
 * 32 bit datatype
 *
 * This typedef defines the 32-bit type used throughout uIP.
 *
 * \hideinitializer
 */
typedef uint32_t u32_t;

/**
 * Statistics datatype
 *
 * This typedef defines the dataype used for keeping statistics in
 * uIP.
 *
 * \hideinitializer
 */
typedef unsigned short uip_stats_t;

/**
 * Maximum number of TCP connections.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_CONNECTIONS 0

/**
 * Maximum number of listening TCP ports.
 *
 * \hideinitializer
 */
#define UIP_CONF_MAX_LISTENPORTS 0

/**
 * uIP buffer size.
 *
 * \hideinitializer
 */
#define UIP_CONF_BUFFER_SIZE     1000
/**
 * CPU byte order.
 *
 * \hideinitializer
 */
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN

/**
 * Logging on or off
 *
 * \hideinitializer
 */
#define UIP_CONF_LOGGING         0

/**
 * UDP support on or off requires DHCP if ON
 *
 * \hideinitializer
 */
#define UIP_CONF_UDP             1

/**
 * UDP checksums on or off
 *
 * \hideinitializer
 */
#define UIP_CONF_UDP_CHECKSUMS   0

/**
 * uIP statistics on or off
 *
 * \hideinitializer
 */
#define UIP_CONF_STATISTICS      0

/**
 * Broadcast support. Needed for dhcp
 *
 * \hideinitializer
 */
#define UIP_CONF_BROADCAST		1

/**
 * The maximum amount of concurrent UDP connections.
 *
 * \hideinitializer
 */
#define UIP_CONF_UDP_CONNS		5

/**
 *  Turn on IP packet re-assembly.
 *  This will re-assemble ip packets that become fragmented
 *    which when assembled will fit in your UIP BUFFER.
 *
 *  Note: This will double the amount of RAM used by the UIP BUFFER.
 */
#define UIP_REASSEMBLY 0


//Include app configuration
#include "apps-conf.h"

#endif /* __UIP_CONF_H__ */

/** @} */
/** @} */
