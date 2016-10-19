/**
 * \file
 *
 * \brief KS8851SNL driver for SAM0.
 *
 * Copyright (c) 2012-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include "asf.h"
#undef F_CPU
#include <util/delay.h>

#include "global-conf.h"
#include "ksz8851snl_reg.h"
#include "ksz8851.h"
#include "net.h"

static uint16_t rxBytesToRead = 0;
static uint16_t rxPaddingBytes = 0;
static uint16_t txBytesWritten = 0;
static uint8_t frameID = 0;


#define enableSpiSS()	\
do                      \
{                       \
    spi_select_device(&SPIC, &spiKsz8851Device);   \
    _delay_us(1);                                   \
} while (0);

#define disableSpiSS()	\
do                      \
{                       \
    _delay_us(1);       \
    spi_deselect_device(&SPIC, &spiKsz8851Device);  \
} while (0);


static struct spi_device spiKsz8851Device = { .id = PIN_ETH_CS };
static SPI_t *spiMaster = &SPIC;


static inline uint8_t SPI_MasterTransceiveByte( SPI_t *spi, uint8_t data )
{
	spi_write_single( spi, data );
    
    // wait until transfer is complete
    while( 0 == spi_is_tx_ready( spi ));
	
	// get received byte
	return spi_get( spi );
}


/**
 * \internal
 * \brief Initialize the hardware interface.
 */
static inline void ksz8851snl_interface_init(void)
{
	spi_master_init( &SPIC );
	spi_master_setup_device( &SPIC, &spiKsz8851Device, SPI_MODE_0,
							KSZ8851_BAUDRATE, 0 );
	spi_enable( &SPIC );
}

/**
 * \internal
 * \brief Perform hardware reset of the PHY.
 */
static inline void ksz8851snl_hard_reset(void)
{
	/* Perform hardware reset with respect to the reset timing from the datasheet. */
	ksz8851_reg_setbits(REG_RESET_CTRL, GLOBAL_SOFTWARE_RESET | QMU_SOFTWARE_RESET);
	_delay_ms(10);
	ksz8851_reg_clrbits(REG_RESET_CTRL, GLOBAL_SOFTWARE_RESET | QMU_SOFTWARE_RESET);
	_delay_ms(10);
}

static void ksz8851Overrun(void)
{
    // Flush RX queue on rx overflow
    ksz8851_reg_clrbits(REG_RX_CTRL1, RX_CTRL_ENABLE);
    ksz8851_reg_setbits(REG_RX_CTRL1, RX_CTRL_FLUSH_QUEUE);
    ksz8851_reg_clrbits(REG_RX_CTRL1, RX_CTRL_FLUSH_QUEUE);
    ksz8851_reg_setbits(REG_RX_CTRL1, RX_CTRL_ENABLE);
}

uint16_t ksz8851_fifo_read_begin( void )
{
	static uint8_t rxFrameCount = 0;
	static bool interruptsOff = false;

	if (rxFrameCount == 0) 
	{
        /* Disable Interrupts */
        ksz8851_reg_write(REG_INT_MASK, 0);
		interruptsOff = true;
        
		uint16_t isr = ksz8851_reg_read(REG_INT_STATUS);
        uint16_t handled = 0;
        bool packetAvailable = false;

		if(isr & INT_RX_OVERRUN) 
		{
            handled |= INT_RX_OVERRUN;
			ksz8851Overrun();  
		}
        if(isr & INT_RX_SPI_ERROR)
        {
            dbg("Spi Error occured \r\n");
            handled |= INT_RX_SPI_ERROR;
        }
        if(isr & INT_RX)
        {
            packetAvailable = true;
            handled |= INT_RX;
        }
        if(isr & INT_RX_STOPPED)
        {
            dbg("Rx process stopped\r\n");
            handled |= INT_RX_STOPPED;
        }
        if(isr & INT_TX_STOPPED)
        {
            dbg("Tx process stopped\r\n");
            handled |= INT_TX_STOPPED;
        }
        if(isr & INT_TX)
        {
            dbg("Tx process done\r\n");
            handled |= INT_TX;
        }
        if(isr & INT_RX_WOL_LINKUP)
        {
            handled |= INT_RX_WOL_LINKUP;
        }
        if(isr & INT_PHY)
        {
            if (ksz8851_reg_read(REG_PHY_STATUS) & PHY_LINK_UP)
            {
                setNetworkEvent(NET_EVENT_LINK_UP);
            }
            else
            {
                setNetworkEvent(NET_EVENT_LINK_DOWN);   
            }
            handled |= INT_PHY;
        }
        
        /* Clear the flags */
        ksz8851_reg_write(REG_INT_STATUS, handled);
        
        if(!packetAvailable)
        {
              if(interruptsOff)
			  {
				  /* Enable Interrupts */
				  ksz8851_reg_write(REG_INT_MASK, INT_MASK);
				  interruptsOff = false;
			  }
              return 0;          
        }

		/* Read rx total frame count */
		rxFrameCount = (ksz8851_reg_read(REG_RX_FRAME_CNT_THRES) & RX_FRAME_CNT_MASK) >> 8;

		if (rxFrameCount == 0) 
		{
			if(interruptsOff)
			{
				/* Enable Interrupts */
				ksz8851_reg_write(REG_INT_MASK, INT_MASK);
				interruptsOff = false;
			}
			return 0;
		}
	}

	uint16_t rxFrameHeaderStatus = ksz8851_reg_read(REG_RX_FHR_STATUS);
    /* Read byte count (4-byte CRC included) */
    rxBytesToRead = ksz8851_reg_read(REG_RX_FHR_BYTE_CNT) & RX_BYTE_CNT_MASK;
    rxPaddingBytes = 4 - ((rxBytesToRead-2) & 0x03);  /* Keep 32bit alignment */

//     dbg_info("Bytes to read: %d\r\n", rxBytesToRead);
//     dbg_info("Padding Bytes: %d\r\n", rxPaddingBytes);

	if (!(rxFrameHeaderStatus & RX_VALID)) 
	{
        dbg_info("RX Packet has errors (RXFHSR state: %x)\r\n", rxFrameHeaderStatus);
		/* Packet has errors */
		/* Issue the RELEASE error frame command */
		ksz8851_reg_setbits(REG_RXQ_CMD, RXQ_CMD_FREE_PACKET);
        while(ksz8851_reg_read(REG_RXQ_CMD) & RXQ_CMD_FREE_PACKET);
		
		rxFrameCount--;
		return 0;
	}
	if (rxBytesToRead == 0) 
	{
		/* Issue the RELEASE error frame command */
		ksz8851_reg_setbits(REG_RXQ_CMD, RXQ_CMD_FREE_PACKET);
        while(ksz8851_reg_read(REG_RXQ_CMD) & RXQ_CMD_FREE_PACKET);
        
        dbg("Packet length is 0\r\n");
		
		rxFrameCount--;
		return 0;
	}

	/* Clear rx frame pointer */
	ksz8851_reg_clrbits(REG_RX_ADDR_PTR, ADDR_PTR_MASK);

	/* Enable RXQ read access */
	ksz8851_reg_setbits(REG_RXQ_CMD, RXQ_START);
	
	enableSpiSS();
    
	SPI_MasterTransceiveByte( spiMaster, FIFO_READ );
	
	/* Read 4-byte garbage */
	/* Read 4-byte status word/byte count */
	/* Read 2-byte alignment bytes */
    for(uint_fast8_t i = 0; i < 10; i++)
    {
        SPI_MasterTransceiveByte(spiMaster, 0);
    }       
    /* Remove the first 2 extra bytes consisting of the 2 bytes offset*/
    rxBytesToRead -= 2;
//    dbg_info("\r\nPacket length: %d\r\n", rxBytesToRead - 4); //without 32bit CRC 
	
	rxFrameCount--;
	return rxBytesToRead - 4; // Return packet length without the 4 byte crc, which will be deleted
}


/**
 * \brief Read internal fifo buffer.
 *
 * \param buf the buffer to store the data from the fifo buffer.
 * \param len the amount of data to read.
 */
void ksz8851_fifo_read(uint8_t *buf, uint16_t packetLength)
{
	/* Perform SPI transfer. */
	for( uint16_t i = 0; i < packetLength; i++ )
	{
		buf[i] = SPI_MasterTransceiveByte( spiMaster, 0 );
	}
    
    rxBytesToRead -= packetLength;
}


void ksz8851_fifo_read_end(void)
{
    /* Keep internal memory alignment. (Read after CRC) */
    /* And Read 4-byte crc */	   
    uint16_t bytesToRead = rxBytesToRead + rxPaddingBytes;
	
    for(uint16_t i = 0; i < bytesToRead; i++)
    {
        SPI_MasterTransceiveByte(spiMaster, 0);
    }
    
	disableSpiSS();

	/* End RXQ read access */
	ksz8851_reg_clrbits(REG_RXQ_CMD, RXQ_START);
}

/**
 * \brief Start to write internal FIFO buffer.
 *
 * \param tot_len the total amount of data to write.
 */
void ksz8851_fifo_write_begin(uint16_t packetLength)
{
   /* Check if TXQ memory size is available for this transmit packet */
   volatile uint16_t txmir = ksz8851_reg_read(REG_TX_MEM_INFO) & TX_MEM_AVAILABLE_MASK;
   
   /* Not enough space to send packet. */
   if (txmir < (packetLength + 4)) 
   {
		/* Enable TXQ memory available monitor */
		ksz8851_reg_write(REG_TX_TOTAL_FRAME_SIZE, packetLength + 4);
        barrier();
		ksz8851_reg_setbits(REG_TXQ_CMD, TXQ_MEM_AVAILABLE_INT);

		/* When the isr register has the TXSAIS bit set, there's
		* enough room for the packet.
		*/
        volatile uint16_t interruptStatus;
        do
        {
            interruptStatus = ksz8851_reg_read(REG_INT_STATUS);
        } 
        while(!(interruptStatus & INT_TX));      
	
		/* Disable TXQ memory available monitor */
		ksz8851_reg_clrbits(REG_TXQ_CMD, TXQ_MEM_AVAILABLE_INT);

		/* Clear the flag */
		ksz8851_reg_setbits(REG_INT_STATUS, INT_TX_SPACE);
	}
    
    /* Disable ALL interrupts */
    ksz8851_reg_write(REG_INT_MASK, 0);
	
	/* Enable TXQ write access */
	ksz8851_reg_setbits(REG_RXQ_CMD, RXQ_START);

	enableSpiSS();
    
    frameID++;
    frameID &= 0x3f;

	/* Prepare control word and byte count. */
	SPI_MasterTransceiveByte(spiMaster, FIFO_WRITE );
	SPI_MasterTransceiveByte(spiMaster, frameID);
	SPI_MasterTransceiveByte(spiMaster, 0);
	SPI_MasterTransceiveByte(spiMaster, packetLength & 0xff);
	SPI_MasterTransceiveByte(spiMaster, packetLength >> 8);
}

/**
 * \brief Write internal fifo buffer.
 *
 * \param buf the buffer to send to the fifo buffer.
 * \param len the size of the pbuf element to write.
 */
void ksz8851_fifo_write(uint8_t *buf, uint16_t packetLength)
{
	/* Perform SPI transfer. */
	for(txBytesWritten = 0; txBytesWritten < packetLength; txBytesWritten++)
	{
		SPI_MasterTransceiveByte(spiMaster, buf[txBytesWritten]);
	}
}

/**
 * \brief Complete write operation.
 *
 * \param pad amount of dummy data (bytes) to write to keep 32 bits alignment
 * in the internal FIFO.
 */
void ksz8851_fifo_write_end( void )
{
	/* Calculate how many bytes to get to DWORD */
	uint16_t txPaddingBytes = 4 - (txBytesWritten & 0x03);

	/* Finish SPI FIFO_WR transaction */
	for(; txPaddingBytes > 0; txPaddingBytes--)
	{
		SPI_MasterTransceiveByte( spiMaster, 0 );
	}
	
	disableSpiSS();

	/* Disable TXQ write access */
	ksz8851_reg_clrbits(REG_RXQ_CMD, RXQ_START);

	/* Issue transmit command to the TXQ */
	ksz8851_reg_setbits(REG_TXQ_CMD, TXQ_ENQUEUE);
    
    /* Wait until transmit command clears */
    while (ksz8851_reg_read(REG_TXQ_CMD) & TXQ_ENQUEUE);
    
    /* Enable interrupts again */
    ksz8851_reg_write(REG_INT_MASK, INT_MASK);
}

/**
 * \brief Read a register value.
 *
 * \param reg the register address to modify.
 *
 * \return the register value.
 */
uint16_t ksz8851_reg_read(uint16_t reg)
{
	uint16_t regData;
	uint16_t cmd;
    
	/* Move register address to cmd bits 9-2, make 32-bit address. */
	cmd = (reg << 2) & REG_ADDR_MASK;

	/* Last 2 bits still under "don't care bits" handled with byte enable. */
	/* Select byte enable for command. */
	if (reg & 2) {
		/* Odd word address writes bytes 2 and 3 */
		cmd |= (0xc << 10);
	} else {
		/* Even word address write bytes 0 and 1 */
		cmd |= (0x3 << 10);
	}

	/* Add command read code. */
	cmd |= CMD_READ;
    
    enableSpiSS();
	SPI_MasterTransceiveByte( spiMaster, cmd >> 8);
	SPI_MasterTransceiveByte( spiMaster, cmd & 0xff);
	regData = (uint16_t)SPI_MasterTransceiveByte( spiMaster, 0xff);
	regData |= (uint16_t)SPI_MasterTransceiveByte( spiMaster, 0xff) << 8;
	disableSpiSS();

	return regData;
}

/**
 * \brief Write a register value.
 *
 * \param reg the register address to modify.
 * \param wrdata the new register value.
 */
void ksz8851_reg_write(uint16_t reg, uint16_t wrdata)
{
	uint16_t cmd;

	/* Move register address to cmd bits 9-
	2, make 32-bit address. */
	cmd = (reg << 2) & REG_ADDR_MASK;

	/* Last 2 bits still under "don't care bits" handled with byte enable. */
	/* Select byte enable for command. */
	if (reg & 2) {
		/* Odd word address writes bytes 2 and 3 */
		cmd |= (0xc << 10);
	} else {
		/* Even word address write bytes 0 and 1 */
		cmd |= (0x3 << 10);
	}

	/* Add command write code. */
	cmd |= CMD_WRITE;
    
    enableSpiSS();
	SPI_MasterTransceiveByte( spiMaster, cmd >> 8 );
	SPI_MasterTransceiveByte( spiMaster, cmd & 0xff);
	SPI_MasterTransceiveByte( spiMaster, wrdata & 0xff);
	SPI_MasterTransceiveByte( spiMaster, wrdata >> 8);
	disableSpiSS();
}

/**
 * \brief Read register content, set bitmask and write back to register.
 *
 * \param reg the register address to modify.
 * \param bits_to_set bitmask to apply.
 */
void ksz8851_reg_setbits(uint16_t reg, uint16_t bits_to_set)
{
   ksz8851_reg_write(reg, ksz8851_reg_read(reg) | bits_to_set);
}

/**
 * \brief Read register content, clear bitmask and write back to register.
 *
 * \param reg the register address to modify.
 * \param bits_to_set bitmask to apply.
 */
void ksz8851_reg_clrbits(uint16_t reg, uint16_t bits_to_clr)
{
   ksz8851_reg_write(reg, ksz8851_reg_read(reg) & ~bits_to_clr);
}


/**
 * \brief Initialize the PHY controller
 *
 * Call this function to initialize the hardware interface and the PHY
 * controller. When initialization is done the PHY is turned on and ready
 * to receive data.
 */
bool ksz8851snl_init(const uint8_t *mac)
{
	uint32_t count = 0;
	uint16_t dev_id;

	/* Initialize the SPI interface. */
	ksz8851snl_interface_init();

	/* Reset the Micrel in a proper state. */
	do {
		ksz8851snl_hard_reset();

		/* Init step1: read chip ID. */
		dev_id = ksz8851_reg_read(REG_CHIP_ID);
		
		dbg_info("Read device ksz8851 DEV ID: %#x\r\n", dev_id & CHIP_ID_MASK );
		if (++count > 10)
			return false;
	} while ((dev_id & CHIP_ID_MASK) != CHIP_ID_8851_SNL);
    
    uint16_t power_reg = ksz8851_reg_read(REG_POWER_CNTL);
    power_reg &= ~POWER_STATE_MASK;
    power_reg |= POWER_STATE_D0;
    ksz8851_reg_write(REG_POWER_CNTL, power_reg);
    
    ksz8851_reg_write(REG_WAKEUP_TIME, 0x01FF);
    

	/* Init step2-4: write QMU MAC address (low, middle then high). */
	ksz8851_reg_write(REG_MAC_ADDR_0, (mac[4] << 8) | mac[5]);
	ksz8851_reg_write(REG_MAC_ADDR_2, (mac[2] << 8) | mac[3]);
	ksz8851_reg_write(REG_MAC_ADDR_4, (mac[0] << 8) | mac[1]);

	/* Init step5: enable QMU Transmit Frame Data Pointer Auto Increment. */
	ksz8851_reg_write(REG_TX_ADDR_PTR, ADDR_PTR_AUTO_INC);

	/* Init step6: configure QMU transmit control register. */
	ksz8851_reg_write(REG_TX_CTRL,
			TX_CTRL_ICMP_CHECKSUM |
			TX_CTRL_UDP_CHECKSUM |
			TX_CTRL_TCP_CHECKSUM |
			TX_CTRL_IP_CHECKSUM |
			TX_CTRL_FLOW_ENABLE |
			TX_CTRL_PAD_ENABLE |
			TX_CTRL_CRC_ENABLE );

	/* Init step7: enable QMU Receive Frame Data Pointer Auto Increment. */
	ksz8851_reg_write(REG_RX_ADDR_PTR, ADDR_PTR_AUTO_INC);

	/* Init step8: configure QMU Receive Frame Threshold for one frame. */
	ksz8851_reg_write(REG_RX_FRAME_CNT_THRES, 1);

	/* Init step9: configure QMU receive control register1. */
	ksz8851_reg_write(REG_RX_CTRL1,
  			RX_CTRL_UDP_CHECKSUM |
  			RX_CTRL_TCP_CHECKSUM |
  			RX_CTRL_IP_CHECKSUM |
			RX_CTRL_MAC_FILTER |
			RX_CTRL_FLOW_ENABLE |
			RX_CTRL_BROADCAST |
			RX_CTRL_ALL_MULTICAST|
			RX_CTRL_UNICAST);

	/* Init step10: configure QMU receive control register2. */
	ksz8851_reg_write(REG_RX_CTRL2,
  			RX_CTRL_IPV6_UDP_NOCHECKSUM |
 			RX_CTRL_UDP_LITE_CHECKSUM |
            RX_CTRL_ICMP_CHECKSUM |
			RX_CTRL_BURST_LEN_FRAME);

	/* Init step11: configure QMU receive queue: trigger INT and auto-dequeue frame. */
	ksz8851_reg_write(REG_RXQ_CMD, RXQ_CMD_CNTL | RXQ_TWOBYTE_OFFSET);

// 	/* Init step12: adjust SPI data output delay. */
// 	ksz8851_reg_write(REG_BUS_CLOCK_CTRL, 1);

	/* Init step13: restart auto-negotiation. */
	ksz8851_reg_setbits(REG_PORT_CTRL, PORT_AUTO_NEG_RESTART);

	/* Init step13.1: force link in half duplex if auto-negotiation failed. */
	if ((ksz8851_reg_read(REG_PORT_CTRL) & PORT_AUTO_NEG_RESTART) != PORT_AUTO_NEG_RESTART)
	{
		ksz8851_reg_clrbits(REG_PORT_CTRL, PORT_FORCE_FULL_DUPLEX);
	}

	/* Init step14: clear interrupt status. */
	ksz8851_reg_write(REG_INT_STATUS, 0xFFFF);

 	/* Init step15: set interrupt mask. */
 	ksz8851_reg_write(REG_INT_MASK, INT_MASK);

	/* Init step16: enable QMU Transmit. */
	ksz8851_reg_setbits(REG_TX_CTRL, TX_CTRL_ENABLE);
    
	/* Init step17: enable QMU Receive. */
	ksz8851_reg_setbits(REG_RX_CTRL1, RX_CTRL_ENABLE);

	return true;
}



void ksz8851_read_mib_counters(void)
{
    dbg("\r\n");
    dbg("=====================================================\r\n");
    dbg("  + Reading Management Information Base Counters + \r\n");
    dbg("=====================================================\r\n");
    
    
    for(uint16_t addr = MIB_POS_RX_BYTE; addr <= MIB_POS_TX_MULTIPLE_COLLISION; addr++)
    {
        if(addr == MIB_POS_RESERVED)
        {
            continue;
        }
        
        ksz8851_reg_write(REG_IND_IACR, (uint16_t)TABLE_MIB|TABLE_READ|addr);
        uint32_t data = ((uint32_t)ksz8851_reg_read(REG_IND_DATA_HIGH) << 16) | ksz8851_reg_read(REG_IND_DATA_LOW);
        
        switch(addr)
        {
            case MIB_POS_RX_BYTE:                   dbg_info("%09d RxByte\r\n", data); break;
            case MIB_POS_RX_UNDERSIZE_PKT:          dbg_info("%09d RxUndersizePkt\r\n", data); break;
            case MIB_POS_RX_FRAGMENTS:              dbg_info("%09d RxFragments\r\n", data); break;
            case MIB_POS_RX_OVERSIZE:               dbg_info("%09d RxOversize\r\n", data); break;
            case MIB_POS_RX_JABBERS:                dbg_info("%09d RxJabbers\r\n", data); break;
            case MIB_POS_RX_SYMBOL_ERROR:           dbg_info("%09d RxSymbolError\r\n", data); break;
            case MIB_POS_RX_CRC_ERROR:              dbg_info("%09d RxCRCError\r\n", data); break;
            case MIB_POS_RX_ALIGNMENT_ERROR:        dbg_info("%09d RxAlignmentError\r\n", data); break;
            case MIB_POS_RX_CONTROL_8808_PKTS:      dbg_info("%09d RxControl8808Pkts\r\n", data); break;
            case MIB_POS_RX_PAUSE_PKTS:             dbg_info("%09d RxPausePkts\r\n", data); break;
            case MIB_POS_RX_BROADCAST:              dbg_info("%09d RxBroadcast\r\n", data); break;
            case MIB_POS_RX_MULTICAST:              dbg_info("%09d RxMulticast\r\n", data); break;
            case MIB_POS_RX_UNICAST:                dbg_info("%09d RxUnicast\r\n", data); break;
            case MIB_POS_RX_64_OCTETS:              dbg_info("%09d Rx64Octets\r\n", data); break;
            case MIB_POS_RX_65_TO_127_OCTETS:       dbg_info("%09d Rx65to127Octets\r\n", data); break;
            case MIB_POS_RX_128_TO_255_OCTETS:      dbg_info("%09d Rx128to255Octets\r\n", data); break;
            case MIB_POS_RX_256_TO_511_OCTETS:      dbg_info("%09d Rx256to511Octets\r\n", data); break;
            case MIB_POS_RX_512_TO_1023_OCTETS:     dbg_info("%09d Rx511to1023Octets\r\n", data); break;
            case MIB_POS_RX_1024_TO_1521_OCTETS:    dbg_info("%09d Rx1024to1521Octets\r\n", data); break;
            case MIB_POS_RX_1522_TO_2000_OCTETS:    dbg_info("%09d Rx1522to2000Octets\r\n", data); break;
            case MIB_POS_TX_BYTE:                   dbg_info("%09d TxByte\r\n", data); break;
            case MIB_POS_TX_LATE_COLLISION:         dbg_info("%09d TxLateCollision\r\n", data); break;
            case MIB_POS_TX_PAUSE_PKTS:             dbg_info("%09d TxPausePkts\r\n", data); break;
            case MIB_POS_TX_BROADCAST_PKTS:         dbg_info("%09d TxBroadcastPkts\r\n", data); break;
            case MIB_POS_TX_MULTICAST_PKTS:         dbg_info("%09d TxMulticastPkts\r\n", data); break;
            case MIB_POS_TX_UNICAST_PKTS:           dbg_info("%09d TxUnicastPkts\r\n", data); break;
            case MIB_POS_TX_DEFERRED:               dbg_info("%09d TxDeferred\r\n", data); break;
            case MIB_POS_TX_TOTAL_COLLISION:        dbg_info("%09d TxTotalCollision\r\n", data); break;
            case MIB_POS_TX_EXCESSIVE_COLLISION:    dbg_info("%09d TxExcessiveCollision\r\n", data); break;
            case MIB_POS_TX_SINGLE_COLLISION:       dbg_info("%09d TxSingleCollision\r\n", data); break;
            case MIB_POS_TX_MULTIPLE_COLLISION:     dbg_info("%09d TxMultipleCollision\r\n", data); break;
            default: 
                break;
        }
    }
    
    
    ksz8851_reg_clrbits(REG_IND_IACR, (uint16_t)TABLE_READ);
}
