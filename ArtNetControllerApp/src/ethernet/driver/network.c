#include "global-conf.h"
#include "ksz8851.h"
#include <avr/io.h>
#include <util/delay.h>
#include "network.h"
#include "net.h"

uint16_t network_read(void) 
{
    uint16_t len = ksz8851_fifo_read_begin();

    if (len == 0) 
	{
        return 0;
    }
    // Avoid SCADA attack - do not overflow
    else if(len > UIP_BUFSIZE)
    {
        len = UIP_BUFSIZE;
    }        
	
    ksz8851_fifo_read(uip_buf, len);
	ksz8851_fifo_read_end();

    return len;
}


void network_send(void) 
{
    if (getNetworkStatus() & NET_STATE_LINK_IS_OK)
    {
        const uint16_t minimalLength = UIP_LLH_LEN + 40;
        
        ksz8851_fifo_write_begin(uip_len);

        if (uip_len <= minimalLength)
        {
            ksz8851_fifo_write(uip_buf, uip_len);
        }
        else
        {
            ksz8851_fifo_write(uip_buf, minimalLength);
            ksz8851_fifo_write(uip_appdata, uip_len - minimalLength);
        }

        ksz8851_fifo_write_end();
    }
}


void network_init_mac(const uint8_t* macaddr)
{
	//Initialise the device
	ksz8851snl_init(macaddr);
}


void network_get_MAC(struct uip_eth_addr * const mac)
{
    uint16_t	temp;

    // read MAC address registers
    // NOTE: MAC address in KSZ8851 is byte-backward

    /* Read QMU MAC address (low) */
    temp = ksz8851_reg_read(REG_MAC_ADDR_0);
    mac->addr[5] = temp & 0xff;
    mac->addr[4] = (temp >> 8);

    /* Read QMU MAC address (middle) */
    temp = ksz8851_reg_read(REG_MAC_ADDR_2);
    mac->addr[3] = temp & 0xff;
    mac->addr[2] = temp >> 8;

    /* Read QMU MAC address (high) */
    temp = ksz8851_reg_read(REG_MAC_ADDR_4);
    mac->addr[1] = temp & 0xff;
    mac->addr[0] = temp >>  8;
}

void network_set_MAC(const struct uip_eth_addr * const mac)
{
    // write MAC address
    // NOTE: MAC address in KSZ8851 is byte-backward

    /* Write QMU MAC address (low) */
    ksz8851_reg_write(REG_MAC_ADDR_0, (mac->addr[4] << 8) | mac->addr[5]);
    /* Write QMU MAC address (middle) */
    ksz8851_reg_write(REG_MAC_ADDR_2, (mac->addr[2] << 8) | mac->addr[3]);
    /* Write QMU MAC address (high) */
    ksz8851_reg_write(REG_MAC_ADDR_4, (mac->addr[0] << 8) | mac->addr[1]);
}

