/*
 * lighting.c
 *
 * Created: 17.04.2015 18:48:12
 *  Author: Christian
 */ 
#include "lighting.h"

#include <avr/io.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <util/delay.h>
#include "asf.h"

#define WS2812B_BAUDRATE            800000u
#define WS2812B_STARTCODE_TIME_US   60u
#define SIZE_OF_DMX_UNIVERSE        512 
#define SIZE_DATA_FRAME             (SIZE_OF_DMX_UNIVERSE * 2)        // 2 Dmx universe = 320rgb leds

#define WS2812B_FRAME_POS_GREEN 0u
#define WS2812B_FRAME_POS_RED   1u
#define WS2812B_FRAME_POS_BLUE  2u

typedef struct  
{
    volatile uint8_t data[SIZE_DATA_FRAME];
    volatile uint16_t actualBlock;
}tFrameBuffer;

static struct edma_channel_config dmaConfig;
static tFrameBuffer frameBuffer;

static void dmaCallback(edma_channel_status_t status);
static void triggerDmaOutput(void);

void initLighting( void )
{	
    memset(frameBuffer.data, 0, sizeof(frameBuffer.data));
    frameBuffer.actualBlock = 0;

	PORTC.DIRSET = PIN1_bm | PIN3_bm;
    
	// max 400 kHz allowed during initialization
	// 32 MHz / (2 * (19 + 1)) = 800 kHz
	USARTC0.BAUDCTRLA = 20;
	USARTC0.BAUDCTRLB = 0;
	// set spi mode
	USARTC0.CTRLC = USART_CMODE_MSPI_gc | USART_CHSIZE1_bm;
	// enable RX and TX
	USARTC0.CTRLB =  USART_TXEN_bm;
	
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
	edma_channel_set_interrupt_level(&dmaConfig, EDMA_INT_LVL_MED);
	edma_set_callback(EDMA_CH_0, dmaCallback);
	
	edma_channel_set_burst_length(&dmaConfig, EDMA_CH_BURSTLEN_1BYTE_gc);
	edma_channel_set_transfer_count16(&dmaConfig, SIZE_DATA_FRAME - 1);
	edma_channel_set_dest_reload_mode(&dmaConfig, EDMA_CH_RELOAD_TRANSACTION_gc);
	
 	edma_channel_set_dest_dir_mode(&dmaConfig, EDMA_CH_DESTDIR_FIXED_gc);
 	edma_channel_set_destination_address(&dmaConfig, (uint16_t)&USARTC0.DATA);
	
	edma_channel_set_src_dir_mode(&dmaConfig, EDMA_CH_DIR_INC_gc);
	edma_channel_set_source_address(&dmaConfig, (uint16_t) &frameBuffer.data[0]);
	edma_channel_set_trigger_source(&dmaConfig, EDMA_CH_TRIGSRC_USARTC0_DRE_gc);
	
	edma_channel_set_repeat(&dmaConfig);
	
	edma_channel_set_single_shot(&dmaConfig);
	edma_enable(EDMA_CHMODE_STD02_gc);
	
	
	edma_channel_write_config(EDMA_CH_0, &dmaConfig);
	_delay_us(100);
	edma_channel_enable(EDMA_CH_0);
	//edma_channel_trigger_block_transfer( EDMA_CH_0 );
}


static void dmaCallback( edma_channel_status_t status )
{	
	switch( status )
	{
		case EDMA_CH_TRANSFER_COMPLETED:
		{
		    frameBuffer.actualBlock += 64;
		   // if (frameBuffer.actualBlock >= SIZE_DATA_FRAME)
		    {
    		    frameBuffer.actualBlock = 0;
    		    _delay_us(WS2812B_STARTCODE_TIME_US);
		    }
		    
		    edma_channel_set_source_address(&dmaConfig, (uint16_t) &frameBuffer.data[0]);
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


void resetLeds( void )
{
	memset( &frameBuffer.data[0] , 0, SIZE_DATA_FRAME );
}


void setRedLeds(uint8_t intensity)
{
	for (uint_fast16_t i = WS2812B_FRAME_POS_RED; i < SIZE_DATA_FRAME; i += 3)
	{
    	frameBuffer.data[i] = intensity;
	}
}
void setGreenLeds(uint8_t intensity)
{
	for (uint_fast16_t i = WS2812B_FRAME_POS_GREEN; i < SIZE_DATA_FRAME; i += 3)
	{
    	frameBuffer.data[i] = intensity;
	}
}
void setBlueLeds(uint8_t intensity)
{
    for (uint_fast16_t i = WS2812B_FRAME_POS_BLUE; i < SIZE_DATA_FRAME; i += 3)
    {
        frameBuffer.data[i] = intensity;
    }        
}

bool writeFrameBuffer(uint16_t dmxUniverse, uint8_t *data, uint16_t length, uint16_t timeoutMs)
{
    assert(data != NULL);
    
    if (length > SIZE_OF_DMX_UNIVERSE)
    {
        length = SIZE_OF_DMX_UNIVERSE;
    }
    
//     irqflags_t flags =  cpu_irq_save();
//     uint16_t actualBlock = frameBuffer.actualBlock;
//     cpu_irq_restore(flags);
//     
//     if (actualBlock > dmxUniverse && actualBlock < (dmxUniverse + SIZE_OF_DMX_UNIVERSE))
//     {   // Damn fckin dma is in my write space...
//         uint16_t writePos = actualBlock + 64;   // Start in the next block
//         uint16_t writeLen = length - actualBlock - 64;
//         
//         memcpy(&frameBuffer.data[dmxUniverse + writePos], &data[writePos], writeLen);
//         
//         writePos = dmxUniverse;
//         writeLen = SIZE_OF_DMX_UNIVERSE - writeLen;
//         
//         memcpy(&frameBuffer.data[writePos], data, writeLen);
//     }
//     else // Dma is far away --> don't care
//     {
//        memcpy(&frameBuffer.data[dmxUniverse], data, length);
//     }

    return true;
}

