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
#ifndef _ETHER_H
#define _ETHER_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup ksz8851snl_phy_controller_group KSZ8851SNL ethernet PHY driver
 *
 * This is a low level driver for the KSZ8851SNL ethernet PHY through 4-wire SPI.
 * It provides functions for configuring and communicating with the ethernet PHY.
 *
 * Before writing data to the ethernet PHY call \ref ksz8851snl_init() which will set up
 * the physical interface and the PHY. A file named \ref conf_eth.h is needed
 * to define which interface to use.
 * In addition one also need to define the pins
 * \ref KSZ8851SNL_RSTN_PIN, \ref KSZ8851SNL_CS_PIN and \ref KSZ8851SNL_INTN_EIC_PIN and
 * the PHY \ref KSZ8851SNL_CLOCK_SPEED.
 *
 * \warning This driver is not reentrant and can not be used in interrupt\
 * service routines without extra care.
 *
 *
 * An example \ref conf_eth.h file could look like
 * \code
 * // Interface configuration for SAM D20 Xplained Pro
 * #define KSZ8851SNL_SPI                                EXT1_SPI_MODULE
 * #define KSZ8851SNL_SPI_IRQn                           SERCOM0_IRQn
 *
 * // Pins configuration.
 * #define KSZ8851SNL_RSTN_PIN                           EXT1_PIN_6
 * #define KSZ8851SNL_CS_PIN                             EXT1_PIN_15
 * #define KSZ8851SNL_INTN_EIC_CHANNEL                   EXT1_IRQ_INPUT
 * #define KSZ8851SNL_INTN_EIC_PIN                       EXT1_IRQ_PIN
 * #define KSZ8851SNL_INTN_EIC_PIN_MUX                   EXT1_IRQ_PINMUX
 *
 * // SPI settings.
 * #define KSZ8851SNL_SPI_PINMUX_SETTING                 EXT1_SPI_SERCOM_MUX_SETTING
 * #define KSZ8851SNL_SPI_PINMUX_PAD0                    EXT1_SPI_SERCOM_PINMUX_PAD0
 * #define KSZ8851SNL_SPI_PINMUX_PAD1                    PINMUX_UNUSED
 * #define KSZ8851SNL_SPI_PINMUX_PAD2                    EXT1_SPI_SERCOM_PINMUX_PAD2
 * #define KSZ8851SNL_SPI_PINMUX_PAD3                    EXT1_SPI_SERCOM_PINMUX_PAD3
 * #define KSZ8851SNL_CLOCK_SPEED                        10000000UL
 * \endcode
 *
 * \section dependencies Dependencies
 * This driver depends on the following modules:
 * - \ref asfdoc_samd20_port_group for IO port control.
 * - \ref asfdoc_samd20_extint_group for IO port interrupt control.
 * - \ref asfdoc_samd20_system_group for getting system clock speeds for init functions.
 * - \ref asfdoc_samd20_sercom_spi_group for communication with the OLED controller
 * @{
 */

/** @{@} */

#include "ksz8851snl_reg.h"
#include <stdint.h>


#define MIB_POS_RX_BYTE                 0
#define MIB_POS_RESERVED                1
#define MIB_POS_RX_UNDERSIZE_PKT        2
#define MIB_POS_RX_FRAGMENTS            3
#define MIB_POS_RX_OVERSIZE             4
#define MIB_POS_RX_JABBERS              5
#define MIB_POS_RX_SYMBOL_ERROR         6
#define MIB_POS_RX_CRC_ERROR            7
#define MIB_POS_RX_ALIGNMENT_ERROR      8
#define MIB_POS_RX_CONTROL_8808_PKTS    9
#define MIB_POS_RX_PAUSE_PKTS           10
#define MIB_POS_RX_BROADCAST            11
#define MIB_POS_RX_MULTICAST            12
#define MIB_POS_RX_UNICAST              13
#define MIB_POS_RX_64_OCTETS            14
#define MIB_POS_RX_65_TO_127_OCTETS     15
#define MIB_POS_RX_128_TO_255_OCTETS    16
#define MIB_POS_RX_256_TO_511_OCTETS    17
#define MIB_POS_RX_512_TO_1023_OCTETS   18
#define MIB_POS_RX_1024_TO_1521_OCTETS  19
#define MIB_POS_RX_1522_TO_2000_OCTETS  20
#define MIB_POS_TX_BYTE                 21
#define MIB_POS_TX_LATE_COLLISION       22
#define MIB_POS_TX_PAUSE_PKTS           23
#define MIB_POS_TX_BROADCAST_PKTS       24
#define MIB_POS_TX_MULTICAST_PKTS       25
#define MIB_POS_TX_UNICAST_PKTS         26
#define MIB_POS_TX_DEFERRED             27
#define MIB_POS_TX_TOTAL_COLLISION      28
#define MIB_POS_TX_EXCESSIVE_COLLISION  29
#define MIB_POS_TX_SINGLE_COLLISION     30
#define MIB_POS_TX_MULTIPLE_COLLISION   31
#define MIB_NUMBER_OF_COUNTERS          32

#define KSZ8851_BAUDRATE 16000000ul



//! \name PHY register read/write functions
//@{
uint16_t ksz8851_reg_read(uint16_t reg);
void ksz8851_reg_write(uint16_t reg, uint16_t wrdata);
void ksz8851_reg_setbits(uint16_t reg, uint16_t bits_to_set);
void ksz8851_reg_clrbits(uint16_t reg, uint16_t bits_to_clr);
//@}

//! \name PHY FIFO read/write functions
//@{
uint16_t ksz8851_fifo_read_begin( void );
void ksz8851_fifo_read(uint8_t *buf, uint16_t packetLength);
void ksz8851_fifo_read_end(void);
void ksz8851_fifo_write_begin(uint16_t packetLength);
void ksz8851_fifo_write(uint8_t *buf, uint16_t len);
void ksz8851_fifo_write_end( void );
//@}

//! \name Initialization and configuration
//@{
bool ksz8851snl_init(const uint8_t *mac);
//@}
void ksz8851_read_mib_counters(void);
/** @} */



#ifdef __cplusplus
}
#endif

#endif

