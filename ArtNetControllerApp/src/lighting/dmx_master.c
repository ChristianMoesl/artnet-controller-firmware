#include <asf.h>
#if (BOARD == DMX_CONTROLLER)

/**
 *	\file
 *	\brief A DMX 512 Master Interface
 */
#include "lighting.h"
#include "timer.h"

#include "asf.h"
#include <assert.h>
#include <string.h>

#include <util/delay.h>

#define DMX_BAUDRATE                250000
#define DMX_NUM_STOPBITS            2
#define DMX_TIME_BREAK_SIGNAL_US    88
#define DMX_TIME_MARK_AFTER_BREAK   8

#define DMX_LEN_OF_STARTCODE        1
#define DMX_LEN_OF_DATA             512
#define DMX_LEN_OF_FRAME            513

#define DMX_POS_OF_STARTCODE        0
#define DMX_POS_OF_DATA             1

#define DMX_NUMBER_OF_BUFFERS   2
#define DMX_TIME_BETWEEN_OUTPUT_FRAMES_MS   500

#define SIZE_DATA_FRAME 513u
static uint8_t frameBuffer[SIZE_DATA_FRAME];

//Arch dependent variables
static struct timer outputTimer;
static struct edma_channel_config dmaConfig;

//Hardware dependent functions
static void initDmxMaster(void);
static void triggerSingleShotDmxOutput(uint8_t *dmxFrame, uint16_t len);
static inline void setLeds(uint_fast16_t startPos, uint_fast16_t increment, uint8_t intensity);

void initLighting(void)
{
    initDmxMaster();
    
    timer_set(&outputTimer, DMX_TIME_BETWEEN_OUTPUT_FRAMES_MS);
    triggerSingleShotDmxOutput(frameBuffer, sizeof(frameBuffer));
}   

void processLighting(void)
{
    if(timer_expired(&outputTimer))
    {
        timer_set(&outputTimer, DMX_TIME_BETWEEN_OUTPUT_FRAMES_MS);
        triggerSingleShotDmxOutput(frameBuffer, DMX_LEN_OF_FRAME);
    }
}

void setRedLeds( uint8_t intensity )
{
    setLeds(0, 4, intensity);
}

void setGreenLeds( uint8_t intensity )
{
    setLeds(1, 4, intensity);
}

void setBlueLeds( uint8_t intensity )
{
    setLeds(2, 4, intensity);
}

void setWarmWhiteLeds(uint8_t intensity)
{
    setLeds(3, 4, intensity);
}

void writeFrameBuffer(uint16_t dmxUniverse, uint8_t *data, uint16_t length)
{
	if (dmxUniverse == DMX_UNIVERSE1)
	{
		if (length > sizeof(frameBuffer) - 1)
		{
			length = sizeof(frameBuffer) - 1;
		}
		
		memcpy(&frameBuffer[1], data, length);
	}
}

static inline void setLeds(uint_fast16_t startPos, uint_fast16_t increment, uint8_t intensity)
{
    frameBuffer[0] = 0;
    while (startPos < SIZE_DATA_FRAME)
    {
        frameBuffer[startPos + 1] = intensity;
        startPos += increment;
    }
}

static void initDmxMaster(void)
{
    usart_rs232_options_t usartConfig =
    {
        .baudrate = DMX_BAUDRATE,
        .charlength = USART_CHSIZE_8BIT_gc,
        .paritytype = USART_PMODE_DISABLED_gc,
        .stopbits = (DMX_NUM_STOPBITS == 1) ? false : true,
    };
    sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm);
    usart_init_rs232(&USARTD0, &usartConfig);
    usart_rx_disable(&USARTD0);
    usart_put(&USARTD0, 0xFF);  // --> Trigger TX Int complete flag
    
    ioport_set_pin_level(PIN_RS485_ENA, true);
    
    edma_channel_set_burst_length(&dmaConfig, EDMA_CH_BURSTLEN_1BYTE_gc);
    edma_channel_set_dest_reload_mode( &dmaConfig, EDMA_CH_RELOAD_TRANSACTION_gc );
    
    edma_channel_set_dest_dir_mode( &dmaConfig, EDMA_CH_DESTDIR_FIXED_gc );
    edma_channel_set_destination_address( &dmaConfig, (uint16_t)&USARTD0.DATA);
    
    edma_channel_set_src_dir_mode( &dmaConfig, EDMA_CH_DIR_INC_gc );
    edma_channel_set_trigger_source( &dmaConfig, EDMA_CH_TRIGSRC_USARTD0_DRE_gc);
    
    edma_channel_set_single_shot( &dmaConfig );
    edma_enable(EDMA_CHMODE_STD02_gc);
}

static void triggerSingleShotDmxOutput(uint8_t *dmxFrame, uint16_t len)
{
    //Send dmx "BREAK" signal
    while(!usart_tx_is_complete(&USARTD0));
    usart_clear_tx_complete(&USARTD0);

    assert(usart_set_baudrate(&USARTD0, 100000, sysclk_get_cpu_hz()));
    usart_put(&USARTD0, 0);


    while(!usart_tx_is_complete(&USARTD0));
    usart_clear_tx_complete(&USARTD0);

    usart_set_baudrate(&USARTD0, DMX_BAUDRATE, sysclk_get_cpu_hz());
    _delay_us(DMX_TIME_MARK_AFTER_BREAK);
    
    //Start transmitter of the whole dmx frame (startcode + 512 data bytes)
    edma_channel_set_transfer_count16(&dmaConfig, len);
    edma_channel_set_source_address(&dmaConfig, (uint16_t)dmxFrame);
    
    assert(EDMA_CH_READY == edma_channel_write_config(EDMA_CH_0, &dmaConfig));
    edma_channel_enable(EDMA_CH_0);
}

#endif