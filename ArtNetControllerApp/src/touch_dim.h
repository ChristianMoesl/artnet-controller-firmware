/*
 * touch_dim.h
 *
 * Created: 12.06.2015 21:23:16
 *  Author: chris_000
 */ 


#ifndef TOUCH_DIM_H_
#define TOUCH_DIM_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef NDEBUG
//#define TDIM_DEBUG	1
#endif


/*! Constants to define the dimm direction */
#define TDIM_DIMM_DIR_UP	1
#define TDIM_DIMM_DIR_DOWN	2

/*! Defines the initial dimm direction after powering up the lamp */
#define TDIM_CONF_POWERUP_DIMM_DIRECTION	TDIM_DIMM_DIR_UP

enum 
{
	/*! Defines the time intervall [ms] between dimming 1 up or down */
	TDIM_CONF_DIMM_INTERVALL_MS = 32,		
	
	/*! Defines the time [ms] to power ON/OFF */
	TDIM_CONF_DIMM_INTERVALL_POWER_MS = 100,
	
	/*! Default startup level [0-255].
	*  Indicates the Position in the lookup table */
	TDIM_CONF_DEFAULT_STARTUP_INTENSITY = 100,
	
	/*! Minimal Intensity in Dimming mode [1-255].
	 *  Indicates the Position in the lookup table. */
	TDIM_CONF_MIN_POWER_ON_DIMM_INTENSITY = 90,
	
	/**
	 *
	 */
	TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS = (5ul * 60ul * 1000ul),
};

typedef struct 
{
	
	uint_fast8_t mode;
	uint_fast8_t savedLevel;
	uint_fast8_t intensityPreset;
}tTdimNvmStruct;

typedef struct 
{
	tTdimNvmStruct nvm;
	uint_fast8_t state;
	uint_fast8_t dimmDirection;
	uint32_t timePressed;
	uint32_t timeSecondPress;
	uint32_t timeNotPressed;
	uint_fast8_t buttonState;
	const uint16_t *lutWithPwmValues;
	void (*setPwmOutput)(uint16_t value);
	uint16_t (*getLutValue)( uint8_t fadeLevel );
	uint32_t timeUntilMemoryChanged_ms;
	
	uint8_t fadeState;
	uint16_t newFadeLevel;
	uint16_t fadeStep;
	uint16_t actualFadeLevel;
	
}tTdimChannelStruct;


extern volatile uint8_t mode;


void TDIM_initChannel(	tTdimChannelStruct *chan,
						void (*setPwmOutput)(uint16_t value),
						uint16_t (*getLutValue)( uint8_t fadeLevel ),
						bool initNvmMemory );
bool TDIM_isConditionToSaveNvmGiven( tTdimChannelStruct *chan );
void TDIM_resetConditionToSaveNvm( tTdimChannelStruct *chan );

void TDIM_processChannel( tTdimChannelStruct *chan, bool buttonPressed );



#endif /* TOUCH_DIM_H_ */