#include <asf.h>
#if (BOARD == LED_CONTROLLER)

#include "lighting.h"

#include "asf.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <util/delay.h>


#define WS2812B_BAUDRATE            800000u
#define WS2812B_STARTCODE_TIME_US   60u
#define SIZE_OF_DMX_UNIVERSE        512
#define SIZE_OF_MY_RGB_DMX_UNIVERSE (100 * 3)
#define SIZE_OF_LAST_RGB_DMX_UNIVERSE (20 * 3)
#define SIZE_DATA_FRAME             (SIZE_OF_MY_RGB_DMX_UNIVERSE * 3 + SIZE_OF_LAST_RGB_DMX_UNIVERSE)        //  320rgb leds

#define WS2812B_FRAME_POS_GREEN 0u
#define WS2812B_FRAME_POS_RED   1u
#define WS2812B_FRAME_POS_BLUE  2u

static struct edma_channel_config dmaConfig;
static uint8_t frameBuffer[SIZE_DATA_FRAME];

static const uint8_t lutRedLedIntensity[256] PROGMEM =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12,
    13, 13, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 20, 20, 21, 21, 22, 22,
    23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 35, 36,
    36, 37, 38, 39, 40, 40, 41, 42, 43, 44, 44, 45, 46, 47, 48, 49, 50, 51, 51, 52,    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,    73, 75, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 87, 88, 90, 91, 92, 93, 95, 96,    97, 98, 100, 101, 102, 104, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 118, 120, 121, 123,    124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 139, 141, 142, 144, 145, 147, 148, 150, 151, 153,    155, 156, 158, 160, 161, 163, 165, 166, 168, 170, 171, 173, 175, 176, 178, 180, 182, 183, 185, 187,    189, 191, 192, 194, 196, 198, 200, 202, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225,    226, 228, 230, 232, 234, 236, 238, 241, 243, 245, 247, 249, 251, 253, 255,
};
static const uint8_t lutGreenLedIntensity[256] PROGMEM =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3,    3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,    10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16,    16, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 24,    24, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 29, 30, 30, 31, 31, 32, 32, 33,    33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 40, 40, 41, 41, 42, 42, 43, 43,    44, 44, 45, 46, 46, 47, 47, 48, 48, 49, 50, 50, 51, 51, 52, 53, 53, 54, 54, 55,    56, 56, 57, 58, 58, 59, 60, 60, 61, 61, 62, 63, 63, 64, 65, 65, 66, 67, 67, 68,    69, 69, 70, 71, 72, 72, 73, 74, 74, 75, 76, 77, 77, 78, 79, 79, 80, 81, 82, 82,    83, 84, 85, 85, 86, 87, 88, 88, 89, 90, 91, 91, 92, 93, 94, 95, 95, 96, 97, 98,    99, 99, 100, 101, 102, 103, 103, 104, 105, 106, 107, 108, 108, 109, 110, 111, 112, 113, 113, 114,    115, 116, 117, 118, 119, 120, 120, 121, 122, 123, 124, 125, 126, 127, 128,
};
static const uint8_t lutBlueLedIntensity[256] PROGMEM =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5,    5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7,    7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11,    11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14,    15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 19, 19, 19,    19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21,
};


static void dmaCallback( edma_channel_status_t status );


void initLighting( void )
{	

	PORTC.DIRSET = PIN1_bm | PIN3_bm;
	// max 400 kHz allowed during initialization
	// 32 MHz / (2 * (19 + 1)) = 800 kHz
	USARTC0.BAUDCTRLA = 20;
	USARTC0.BAUDCTRLB = 0;
	// set spi mode
	USARTC0.CTRLC = USART_CMODE_MSPI_gc | USART_CHSIZE1_bm;
	// enable RX and TX
	USARTC0.CTRLB = /* USART_RXEN_bm |*/ USART_TXEN_bm;
	

	
	// Setup XCL
	PORTC.DIRSET = PIN0_bm;
	xcl_enable( XCL_ASYNCHRONOUS );
	XCL.CTRLA &= ~XCL_PORTSEL_gm;
	XCL.CTRLA |= XCL_PORTSEL_PC_gc;		/* Setup LUT with output on PC0 */
	xcl_lut_in0( LUT_IN_EVSYS );
	xcl_lut_in1( LUT_IN_EVSYS );
	xcl_lut_in2( LUT_IN_EVSYS );
	xcl_lut_in3( LUT_IN_EVSYS );
	xcl_lut_type( LUT_CONF_MUX );
	xcl_lut1_truth( IN2 );	// pass IN2 
	//xcl_lut0_truth( 0 );	// mux-> ignore
	xcl_lut0_output( LUT0_OUT_PIN0 );
	
	
	// Setup timers for WS2811 waveform generation
	tc45_enable( &TCD5 );
	PORTD.DIRSET = PIN4_bm | PIN5_bm;                                   /* Enable output on PD4 & PD5 for compare channels */
	TCD5.CTRLB = TC45_WGMODE_SINGLESLOPE_gc;                            /* Single Slope PWM */
	TCD5.CTRLD = TC45_EVACT_RESTART_gc | TC45_EVSEL_CH7_gc;             /* Restart on CH7 pulse - rising clock edge */
	TCD5.CTRLE = TC45_CCAMODE_COMP_gc | TC45_CCBMODE_COMP_gc;           /* Enable output compare on CCA & CCB */
	TCD5.PER = 40 - 1;                                                  /* At 32MHz, 1 cycle = 31.25ns.  Define top of counter for a 1250ns pulse: (32MHz / 800KHz) */
	TCD5.CCA = 11;                                                       /* Compare for 0 bit @ 250ns (31.25ns * 8). Output is on PD4 */
	TCD5.CCB = 29;                                                      /* Compare for 1 bit @ 1000ns (31.25ns * 32). Output is on PD5 */
	TCD5.CTRLA = TC45_CLKSEL_DIV1_gc | TC5_EVSTART_bm | TC5_UPSTOP_bm;  /* Start and stop the timer on each event occurrence, full speed clock */
	
	
	
	// Setup event system channels
	EVSYS.CH7MUX = EVSYS_CHMUX_PORTC_PIN1_gc;   /* SPI Clock (PC1) to CH7 */
	PORTC.PIN1CTRL |= PORT_ISC_RISING_gc;       /* Sense rising edge on PC1 */
	EVSYS.CH1MUX = EVSYS_CHMUX_PORTC_PIN3_gc;   /* TXD (PC3) to CH1 / LUT IN2 */
	PORTC.PIN3CTRL |= PORT_ISC_LEVEL_gc;        /* Sense level on PC3 */
	EVSYS.CH6MUX = EVSYS_CHMUX_PORTD_PIN4_gc;   /* CCA (PD4) Compare to CH6 / LUT IN0 */
	PORTD.PIN4CTRL |= PORT_ISC_LEVEL_gc;        /* Sense level on PD4 */
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTD_PIN5_gc;   /* CCB (PD5) Compare to CH0 / LUT IN1 */
	PORTD.PIN5CTRL |= PORT_ISC_LEVEL_gc;        /* Sense level on PD5 */
	
	
	Enable_global_interrupt();
	// Setup dma controller 
	edma_channel_set_interrupt_level( &dmaConfig, EDMA_INT_LVL_MED );
	edma_set_callback( EDMA_CH_0, dmaCallback );
	
	edma_channel_set_burst_length(&dmaConfig, EDMA_CH_BURSTLEN_1BYTE_gc);
	edma_channel_set_transfer_count16( &dmaConfig, SIZE_DATA_FRAME);
	edma_channel_set_dest_reload_mode( &dmaConfig, EDMA_CH_RELOAD_TRANSACTION_gc );
	
 	edma_channel_set_dest_dir_mode( &dmaConfig, EDMA_CH_DESTDIR_FIXED_gc );
 	edma_channel_set_destination_address( &dmaConfig, (uint16_t)&USARTC0.DATA );
	
	edma_channel_set_src_dir_mode( &dmaConfig, EDMA_CH_DIR_INC_gc );
	edma_channel_set_source_address( &dmaConfig, (uint16_t)frameBuffer);
	edma_channel_set_trigger_source( &dmaConfig, EDMA_CH_TRIGSRC_USARTC0_DRE_gc);
	
	edma_channel_set_repeat( &dmaConfig );
	
	edma_channel_set_single_shot( &dmaConfig );
	edma_enable(EDMA_CHMODE_STD02_gc);
	
	
	edma_channel_write_config( EDMA_CH_0, &dmaConfig );
	_delay_us(100);
	edma_channel_enable( EDMA_CH_0 );
	//edma_channel_trigger_block_transfer( EDMA_CH_0 );
}


void processLighting(void)
{
    
}


static void dmaCallback( edma_channel_status_t status )
{	
	switch( status )
	{
		case EDMA_CH_TRANSFER_COMPLETED:
		{
            _delay_us(WS2812B_STARTCODE_TIME_US);
        
            edma_channel_set_source_address(&dmaConfig, (uint16_t)frameBuffer);
            edma_channel_write_config(EDMA_CH_0, &dmaConfig);
            edma_channel_enable(EDMA_CH_0);

		}
		break;
		
		case EDMA_CH_FREE:
		case EDMA_CH_READY:
		case EDMA_CH_PENDING:
		case EDMA_CH_BUSY:
		case EDMA_CH_TRANSFER_ERROR:
		case EDMA_CH_UNAVAILABLE:
		default:
		break;
	}	 
}


void setRedLeds( uint8_t intensity )
{
	for (uint_fast8_t pos = 1; pos < SIZE_DATA_FRAME; pos += 3)
	{
    	frameBuffer[pos] = intensity;
	}
}
void setGreenLeds( uint8_t intensity )
{
	for (uint_fast8_t pos = 0; pos < SIZE_DATA_FRAME; pos += 3)
	{
    	frameBuffer[pos] = intensity;
	}
}
void setBlueLeds( uint8_t intensity )
{
    for (uint_fast8_t pos = 2; pos < SIZE_DATA_FRAME; pos += 3)
    {
        frameBuffer[pos] = intensity;
    }
}

void setWarmWhiteLeds(uint8_t intensity)
{
    setRedLeds(pgm_read_byte(&lutRedLedIntensity[intensity]));
    setGreenLeds(pgm_read_byte(&lutGreenLedIntensity[intensity]));
    setBlueLeds(pgm_read_byte(&lutBlueLedIntensity[intensity]));
}


void writeFrameBuffer(tDmxUniverse dmxUniverse, uint8_t *data, uint16_t length)
{
    assert(data != NULL);
    
    if (length > SIZE_OF_MY_RGB_DMX_UNIVERSE)
    {
        length = SIZE_OF_MY_RGB_DMX_UNIVERSE;
    }
    
	switch (dmxUniverse) 
	{
		case DMX_UNIVERSE1: 
		{
			memcpy(&frameBuffer[SIZE_OF_MY_RGB_DMX_UNIVERSE * 0], data, length);
			break;
		}
		case DMX_UNIVERSE2:
		{
			memcpy(&frameBuffer[SIZE_OF_MY_RGB_DMX_UNIVERSE * 1], data, length);
			break;
		}
		case DMX_UNIVERSE3:
		{
			memcpy(&frameBuffer[SIZE_OF_MY_RGB_DMX_UNIVERSE * 2], data, length);
			break;
		}
		case DMX_UNIVERSE4:
		{
			if (length > SIZE_OF_LAST_RGB_DMX_UNIVERSE) 
			{
				length = SIZE_OF_LAST_RGB_DMX_UNIVERSE;
			}
			memcpy(&frameBuffer[SIZE_OF_MY_RGB_DMX_UNIVERSE * 3], data, length);
			break;
		}
	}
}

#endif