/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <stdbool.h>

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

/*! PORTA initial pin definitions */
#define PIN_INIT_ETH_PME        PIN0_bm
#define PIN_INIT_ETH_INT        PIN1_bm
#define PIN_INIT_U_VIN          PIN2_bm
#define PIN_INIT_I_LED          PIN3_bm
    
#define PORTA_DIR_INIT          0
#define PORTA_PULLUP_INIT       0
    
#define PORTA_OUT_INIT          0

#define PIN_ETH_INT			    IOPORT_CREATE_PIN( PORTA, PIN1_bp )
#define PIN_ETH_PME			    IOPORT_CREATE_PIN( PORTA, PIN0_bp )


/*! PORTC initial pin definitions */
#define PIN_INIT_TX_LED_3_3V    PIN0_bm
#define PIN_INIT_XCK_RESERVED   PIN1_bm
#define PIN_INIT_TXD_RESERVED   PIN3_bm
#define PIN_INIT_ETH_CS         PIN4_bm
#define PIN_INIT_ETH_SCK        PIN5_bm
#define PIN_INIT_ETH_MISO       PIN6_bm
#define PIN_INIT_ETH_MOSI       PIN7_bm
    
#define PORTC_DIR_INIT          (PIN_INIT_TX_LED_3_3V|PIN_INIT_XCK_RESERVED|PIN_INIT_TXD_RESERVED \
                                |PIN_INIT_ETH_CS|PIN_INIT_ETH_SCK|PIN_INIT_ETH_MOSI)
#define PORTC_PULLUP_INIT       0
    
#define PORTC_OUT_INIT          (PIN_INIT_TX_LED_3_3V|PIN_INIT_ETH_CS|PIN_INIT_ETH_SCK|PIN_INIT_ETH_MOSI)

#define PIN_TX_LED_3_3V         IOPORT_CREATE_PIN( PORTC, PIN0_bp )
#define PIN_TXD_RESERVED        IOPORT_CREATE_PIN( PORTC, PIN3_bp )
#define PIN_XCK_RESERVED        IOPORT_CREATE_PIN( PORTC, PIN1_bp )
#define PIN_ETH_CS              IOPORT_CREATE_PIN( PORTC, PIN4_bp )


/*! PORTD initial pin definitions */
#define PIN_INIT_PWM_LOGIC_0    PIN4_bm
#define PIN_INIT_PWM_LOGIC_1    PIN5_bm
#define PIN_INIT_LED_GREEN      PIN6_bm
    
#define PORTD_DIR_INIT          (PIN_INIT_PWM_LOGIC_0|PIN_INIT_PWM_LOGIC_1)
#define PORTD_PULLUP_INIT       PIN_INIT_LED_GREEN
#define PORTD_OUT_INIT          0

#define PIN_PWM_LOGIC_0         IOPORT_CREATE_PIN( PORTD, PIN4_bp )
#define PIN_PWM_LOGIC_1         IOPORT_CREATE_PIN( PORTD, PIN5_bp )
#define PIN_LED_GREEN           IOPORT_CREATE_PIN( PORTD, PIN6_bp )


static inline void enableRs485EnaPin(void)
{
    
}

static inline void disableRs485EnaPin(void)
{
    
}

static inline void setStatusLed(bool isOn)
{
    
}

static inline void setReceiveLed(bool isOn)
{
    
}

#endif // USER_BOARD_H
