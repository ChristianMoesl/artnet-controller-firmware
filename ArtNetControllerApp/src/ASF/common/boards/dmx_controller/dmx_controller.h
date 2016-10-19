
#include <stdbool.h>

#ifndef DMX_CONTROLLER_H_
#define DMX_CONTROLLER_H_

/*! PORTA initial pin definitions */
#define PIN_INIT_U_VIN          PIN0_bm
#define PIN_INIT_I_LED          PIN1_bm

#define PORTA_DIR_INIT          0
#define PORTA_PULLUP_INIT       ~(PIN_INIT_U_VIN|PIN_INIT_I_LED)
#define PORTA_OUT_INIT          0

/*! PORTC initial pin definitions */
#define PIN_INIT_SDA_TXD_EXT    PIN0_bm
#define PIN_INIT_SCL_RXD_EXT    PIN1_bm
#define PIN_INIT_ETH_PME        PIN2_bm
#define PIN_INIT_ETH_INT        PIN3_bm
#define PIN_INIT_ETH_CS         PIN4_bm
#define PIN_INIT_ETH_SCK        PIN5_bm
#define PIN_INIT_ETH_MISO       PIN6_bm
#define PIN_INIT_ETH_MOSI       PIN7_bm

#define PORTC_DIR_INIT          (PIN_INIT_ETH_CS | PIN_INIT_ETH_SCK |PIN_INIT_ETH_MOSI)
#define PORTC_PULLUP_INIT       (PIN_INIT_SDA_TXD_EXT | PIN_INIT_SCL_RXD_EXT)
#define PORTC_OUT_INIT          (PIN_INIT_ETH_CS | PIN_INIT_ETH_SCK | PIN_INIT_ETH_MOSI | PIN_INIT_SDA_TXD_EXT)

#define PIN_SDA_TXD_EXT         IOPORT_CREATE_PIN( PORTC, PIN0_bp )
#define PIN_SCL_RXD_EXT         IOPORT_CREATE_PIN( PORTC, PIN1_bp )
#define PIN_ETH_CS              IOPORT_CREATE_PIN( PORTC, PIN4_bp )

/*! PORTD initial pin definitions */
#define PIN_INIT_RS485_ENA      PIN1_bm
#define PIN_INIT_RS485_RX       PIN2_bm
#define PIN_INIT_RS485_TX       PIN3_bm
#define PIN_INIT_LED_GREEN      PIN4_bm
#define PIN_INIT_LED_RED        PIN5_bm
#define PIN_INIT_ETH_RESET      PIN6_bm

#define PORTD_DIR_INIT          (PIN_INIT_RS485_ENA | PIN_INIT_RS485_TX | PIN_INIT_LED_GREEN | PIN_INIT_LED_RED)
#define PORTD_PULLUP_INIT       (PIN0_bm | PIN7_bm)
#define PORTD_OUT_INIT          (PIN_INIT_RS485_TX)

#define PIN_RS485_ENA           IOPORT_CREATE_PIN( PORTD, PIN1_bp )
#define PIN_RS485_TX            IOPORT_CREATE_PIN( PORTD, PIN3_bp )
#define PIN_LED_GREEN           IOPORT_CREATE_PIN( PORTD, PIN4_bp )
#define PIN_LED_RED             IOPORT_CREATE_PIN( PORTD, PIN5_bp )

static inline void enableRs485EnaPin(void)
{
    PORTD.OUTSET = PIN_INIT_RS485_ENA;
}

static inline void disableRs485EnaPin(void)
{
    PORTD.OUTCLR = PIN_INIT_RS485_ENA;
}

static inline void setStatusLed(bool isOn)
{
    if (isOn)
    {
        PORTD.OUTSET = PIN_INIT_LED_GREEN;
    }
    else
    {
        PORTD.OUTCLR = PIN_INIT_LED_GREEN;
    }
}

static inline void setReceiveLed(bool isOn)
{
    if (isOn)
    {
        PORTD.OUTSET = PIN_INIT_LED_RED;
    }
    else
    {
        PORTD.OUTCLR = PIN_INIT_LED_RED;
    }
}


#endif /* DMX_CONTROLLER_H_ */