#include <asf.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "lighting.h"
#include "net.h"
#include "indicator.h"
#include "artnet.h"
#include "timer.h"
#include "touch_dim.h"
#include "memory.h"

#define POS_TDIM_INTENSITY      0u
#define POS_TDIM_SAVED_LEVEL    1u
#define POS_TDIM_MODE           2u

#define BUTTON_DEBOUNCE_TIME_MS 20u

static struct timer msTickTimer;
static tTdimChannelStruct tdimChannel;

static void setTouchDimOutputValue(uint16_t value);
static uint16_t getTouchDimLutvalue(uint8_t preset);
static bool isButtonPressed(void);

int main(void)
{
    wdt_reset();
	// enable clocks, pinout, interrupt controller
	board_init();
  
	initLighting();
    initIndicators();
    
    wdt_reset();
    initNetwork();
    sei();
    
    setNetworkEvent(NET_EVENT_START_DHCP);
    
    wdt_reset();
    uint8_t eepromData[EEPROM_TOUCH_DIM_LEN];
    
    bool isEepromDataValid = EEPROM_ALL_OK == readEepromStream(EEPROM_TOUCH_DIM_ADDR, eepromData, 
                                                        EEPROM_TOUCH_DIM_LEN, EEPROM_TOUCH_DIM_VERSION);
    if(!isEepromDataValid)
    {
        tdimChannel.nvm.intensityPreset = eepromData[POS_TDIM_INTENSITY];
        tdimChannel.nvm.savedLevel = eepromData[POS_TDIM_SAVED_LEVEL];
        tdimChannel.nvm.mode = eepromData[POS_TDIM_MODE];
    }
    
    TDIM_initChannel(&tdimChannel, setTouchDimOutputValue, getTouchDimLutvalue, !isEepromDataValid);
    timer_set(&msTickTimer, 1);
    
	while(true)
	{
        wdt_reset();
        
        processNetwork();
        processLighting();
        processIndicators();

        if(timer_expired(&msTickTimer))
        {
            timer_restart(&msTickTimer);
            
            static uint8_t timeButtonUnpressed = BUTTON_DEBOUNCE_TIME_MS;
            
            if (timeButtonUnpressed != UINT8_MAX)
            {
                timeButtonUnpressed++;
            }
            if (isButtonPressed())
            {
                timeButtonUnpressed = 0;
            }
            
            TDIM_processChannel(&tdimChannel, false);//timeButtonUnpressed < BUTTON_DEBOUNCE_TIME_MS);
            
            if(TDIM_isConditionToSaveNvmGiven(&tdimChannel))
            {
                uint8_t bufferToSave[EEPROM_TOUCH_DIM_LEN];

                bufferToSave[POS_TDIM_INTENSITY] = tdimChannel.nvm.intensityPreset;
                bufferToSave[POS_TDIM_MODE] = tdimChannel.nvm.mode;
                bufferToSave[POS_TDIM_SAVED_LEVEL] = tdimChannel.nvm.savedLevel;
                writeEepromStream(EEPROM_TOUCH_DIM_ADDR, bufferToSave, EEPROM_TOUCH_DIM_LEN, EEPROM_TOUCH_DIM_VERSION);
                TDIM_resetConditionToSaveNvm(&tdimChannel);
            }
        }
	}
}

static bool isButtonPressed(void)
{
    #if BOARD == DMX_CONTROLLER
    return !ioport_get_pin_level(PIN_SCL_RXD_EXT) || !ioport_get_pin_level(PIN_SDA_TXD_EXT);
    #else
    return !ioport_get_pin_level(PIN_LED_GREEN);
    #endif
}

static void setTouchDimOutputValue(uint16_t value)
{
//    assert(value <= UINT8_MAX);
    setWarmWhiteLeds(value);
}

static uint16_t getTouchDimLutvalue(uint8_t preset)
{
    return preset;
}
