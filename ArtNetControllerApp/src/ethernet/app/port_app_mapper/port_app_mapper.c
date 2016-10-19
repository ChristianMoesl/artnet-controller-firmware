#include <string.h>
#include <assert.h>

#include "uip.h"
#include "uiplib.h"
#include "port_app_mapper.h"
#include "global-conf.h"


#ifndef WEBSERVER_APP_CALL_MAP
#define WEBSERVER_APP_CALL_MAP
#endif

#ifndef WEBCLIENT_APP_CALL_MAP
#define WEBCLIENT_APP_CALL_MAP
#endif

#ifndef TELNET_APP_CALL_MAP
#define TELNET_APP_CALL_MAP
#endif

#ifndef SIMPLE_HTTPD_APP_CALL_MAP
#define SIMPLE_HTTPD_APP_CALL_MAP
#endif

#ifndef DHCPC_APP_CALL_MAP
#define DHCPC_APP_CALL_MAP
#endif

#ifndef RESOLV_APP_CALL_MAP
#define RESOLV_APP_CALL_MAP
#endif

#ifndef NTPCLIENT_APP_CALL_MAP
#define NTPCLIENT_APP_CALL_MAP
#endif

#ifndef ARTNET_APP_CALL_MAP
#define ARTNET_APP_CALL_MAP
#endif

#ifndef MY_PROTOCOL_APP_CALL_MAP
#define MY_PROTOCOL_APP_CALL_MAP
#endif

struct port_appcall_map tcp_port_app_map[] = {
    WEBSERVER_APP_CALL_MAP
    WEBCLIENT_APP_CALL_MAP
    TELNET_APP_CALL_MAP
    SIMPLE_HTTPD_APP_CALL_MAP
    {NULL, 0, 0},
};

struct port_appcall_map udp_port_app_map[] = {
    DHCPC_APP_CALL_MAP
    ARTNET_APP_CALL_MAP
    MY_PROTOCOL_APP_CALL_MAP
    RESOLV_APP_CALL_MAP
    NTPCLIENT_APP_CALL_MAP
    {NULL, 0, 0},
};

// define a basic common type for udp and tcp connections so I can look at the ports
struct uip_base_conn {
  uip_ipaddr_t ripaddr;   /**< The IP address of the remote host. */

  u16_t lport;        /**< The local port, in network byte order. */
  u16_t rport; 
};

struct uip_base_conn *base_conn;

void port_app_mapper_init(void)
{
    for(uint8_t i = 0; i < UIP_CONNS; i++)
    {
        uip_conns[i].appstate = PORT_APP_MAPPER_APPSTATE_UNALLOCATED;
    }
    for(uint8_t i = 0; i < UIP_UDP_CONNS; i++)
    {
        uip_udp_conns[i].appstate = PORT_APP_MAPPER_APPSTATE_UNALLOCATED;
    }
}

bool port_app_mapper_change_udp_port(void (*appCall)(void), uint16_t lport, uint16_t rport)
{   
    const int_fast8_t numberOfAppEntries = (sizeof(udp_port_app_map) / sizeof(struct port_appcall_map)) - 1;
}
    
    
    
static bool change_port(struct port_appcall_map* map, void (*appCall)(void), uint16_t lport, uint16_t rport)
{  /*  
    int_fast8_t appIndexToChange = -1;
    
    for (int_fast8_t i = 0; map[i].an_appcall != NULL; i++)
    {
        if (map[i].an_appcall == appCall)
        {
            appIndexToChange = i;
        }
        else if ((lport != 0)&&(map[i].lport == lport))        // Port is already in use
        {
            return false;
        }
        else if ((rport != 0)&&(map[i].rport == rport))
    }
    if (appIndexToChange >= 0)
    {
        uint16_t oldLPort = htons(udp_port_app_map[appIndexToChange].lport);
        uint16_t oldRPort = htons(udp_port_app_map[appIndexToChange].rport);
        
        for (uint8_t i = 0; i < UIP_UDP_CONNS; i++)
        {
            if (((uip_udp_conns[i].rport == oldRPort) && ((oldLPort == 0) || (uip_udp_conns[i].lport == oldLPort))) 
            ||  ((uip_udp_conns[i].lport == oldLPort) && ((oldRPort == 0) || (uip_udp_conns[i].rport == oldRPort))))
            {
                if (rport != 0)
                {
                    uip_udp_conns[i].rport = rport;
                }
                if (lport != 0)
                {
                    uip_udp_conns[i].lport = lport; 
                }
            }
        }
        udp_port_app_map[appIndexToChange].lport = lport;
        udp_port_app_map[appIndexToChange].rport = rport;            
            
            
            if ((uip_udp_conns[i]->rport == oldRPort &&
                (oldLPort == 0 || uip_udp_conns[i]->lport == oldLPort)) 
            {
                    
                    
            }
//                 ((uip_udp_conns[i]->lport == oldLPort) &&
//                 ((oldRPort == 0) || (uip_udp_conns[i]->rport == HTONS(cur_map->rport)))))
            {
                    
            }
            
            // Search for connections used by this app
            if (uip_udp_conns[i].lport == nodeStatus.networkPort
                    && uip_udp_conns[i].appstate >= 0
                        && uip_udp_conns[i].appstate < ART_NET_MAX_NUMBER_OF_CONN_APP_STATES)
            {
                
                
                // Remap only active connections
                if (appStateList[uip_udp_conns[i].appstate].commStatus & ART_NET_COMM_STATUS_CONNECTED)
                {
                    uip_udp_bind(&uip_udp_conns[i], htons(newPort));
                }
            }
        }
        
        
        
        return true;    // successfully mapped the port
    }
    else
    {
        return false;   // The desired app is not found in the list
    }*/
    return false;
}



// this function maps the current packet against the mappings
//   pointed to by the param
void uip_port_app_mapper(struct port_appcall_map* cur_map)
{
#ifdef __DHCPC_H__
    // if dhcpc is enabled and running we want our ip ASAP so skip all others
   if(dhcpc_running && uip_poll())
   {
       if (base_conn != NULL)
       {
           if (base_conn->lport == HTONS(DHCPC_CLIENT_PORT) && base_conn->rport == HTONS(DHCPC_SERVER_PORT))
           {
               dhcpc_appcall();
               base_conn = NULL;
           }
       }
       return;
   }
#endif

    // yes this will walk the entire list which is up to 4 items at the moment
    while ((base_conn != NULL) && (cur_map->an_appcall != NULL))
    {
        // Now match the app to the packet.
        // local AND/OR remote ports match
        // firs check remote port matches and local may or may not
        // then check local port matches
        // can't do it in one statement due to the l & r ports could both be zero and match all apps.
        
        if (((base_conn->rport == HTONS(cur_map->rport)) &&
            ((cur_map->lport == 0) || (base_conn->lport == HTONS(cur_map->lport)))) ||
            ((base_conn->lport == HTONS(cur_map->lport)) &&
             ((cur_map->rport == 0) || (base_conn->rport == HTONS(cur_map->rport)))))
        {
            cur_map->an_appcall();
            base_conn = NULL;
        }
        cur_map++;
    }
}


// wrapper around the uip_port_app_mapper
// pass it the list of apps to match against
void uip_udp_appcall_port_app_mapper(void)
{
    base_conn = (struct uip_base_conn *) uip_udp_conn;
    uip_port_app_mapper(udp_port_app_map);
}

void uip_appcall_port_app_mapper(void)
{
    base_conn = (struct uip_base_conn *) uip_conn;
    uip_port_app_mapper(tcp_port_app_map);
}
