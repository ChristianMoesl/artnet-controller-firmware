/*
 * touch_dim.c
 *
 * Created: 12.06.2015 21:23:30
 *  Author: chris_000
 */ 
#include "touch_dim.h"
#include "lighting.h"

#include <assert.h>

enum /*! Touch dim state definitions */
{
	STATE_INIT,
	STATE_IDLE,
	STATE_PROCESS_DIMMING,
	STATE_POWER_CONTROL,
	STATE_PROCESS_MODE,
};
enum /*! Button state definitions */
{
	BTN_STATE_IDLE,
	BTN_STATE_WAITING_FOR_DOUBLE_PRESS,
	BTN_STATE_SHORT_PRESS_DETECTED,
	BTN_STATE_LONG_PRESS_DETECTED,
	BTN_STATE_DOUBLE_PRESS_DETECTED,
	BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED,
	BTN_STATE_RESET,
};
enum /*! Dimm direction definitions */
{
	DIMM_DIR_UP,
	DIMM_DIR_DOWN,
};
enum /*! Touch dim modes */
{
	TDIM_MODE1_LAST_DIMM_VALUE,
	TDIM_MODE2_SAVED_DIMM_VALUE,
	
	TDIM_MODE_DEFAULT = TDIM_MODE2_SAVED_DIMM_VALUE,
};
enum /*! Intensity limits */
{
	INTENSITY_OFF			= 0,
	INTENSITY_MIN			= TDIM_CONF_MIN_POWER_ON_DIMM_INTENSITY,
	INTENSITY_MAX			= 200,
};
enum /* Timing definitions in [ms] */
{
	TIME_THRESHOLD_BETWEEN_SHORT_AND_LONG_PRESS = 500,
	TIME_THRESHOLD_DOUBLE_PRESS = 400,
	TIME_THRESHOLD_DEBOUNCE_ON_SHORT_PRESS = 20,
};
enum /* Timing definitions for nvm [s] */
{	
	TIME_MEMORY_UP2DATE_MS = UINT16_MAX,
};
enum
{
	FADE_SHIFT_VALUE = 8,
	FADE_RESOLUTION = 1 << FADE_SHIFT_VALUE,
	FADE_FRACTION_MASK = FADE_RESOLUTION-1,
	FADE_STEP_SIZE = FADE_RESOLUTION / TDIM_CONF_DIMM_INTERVALL_POWER_MS,
	
	MAX_FADE_LEVEL_VALUE = 255 << FADE_SHIFT_VALUE,
	MIN_FADE_LEVEL_VALUE = 0   << FADE_SHIFT_VALUE,
};
enum FadeState
{
	FADE_STATE_IDLE,
	FADE_STATE_UP,
	FADE_STATE_DOWN,
};


#if( TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS == UINT16_MAX )
#error TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS should be < UINT16_MAX
#endif

static void setNewFadeLevel( tTdimChannelStruct *chan );
static bool fadeDown( tTdimChannelStruct *chan );
static bool fadeUp( tTdimChannelStruct *chan );


static inline void processButton( tTdimChannelStruct *chan, bool buttonPressed  );


static inline void resetMemoryTimer( tTdimChannelStruct *chan )
{
	if( chan->timeUntilMemoryChanged_ms != TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS )
	{
		chan->timeUntilMemoryChanged_ms = 0;
	}
}


void TDIM_initChannel(	tTdimChannelStruct *chan,
						void (*setPwmOutput)(uint16_t value),
						uint16_t (*getLutValue)( uint8_t fadeLevel ),
						bool initNvmMemory )
{	
	assert( NULL != chan );
	assert( NULL != setPwmOutput );
	assert( NULL != getLutValue );
	
	if( initNvmMemory )
	{
		chan->nvm.savedLevel = TDIM_CONF_DEFAULT_STARTUP_INTENSITY;
		chan->nvm.mode = TDIM_MODE_DEFAULT;
		chan->nvm.intensityPreset = INTENSITY_OFF;
		chan->timeUntilMemoryChanged_ms = TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS;
	}
	else
	{
		chan->timeUntilMemoryChanged_ms = 0;
	}
	
	chan->setPwmOutput = setPwmOutput;
	chan->getLutValue = getLutValue;
	chan->buttonState = BTN_STATE_RESET;
	chan->state = STATE_INIT;
	chan->dimmDirection = DIMM_DIR_UP;		
	
	chan->fadeState = FADE_STATE_IDLE;
	chan->actualFadeLevel = 0;
}




bool TDIM_isConditionToSaveNvmGiven( tTdimChannelStruct *chan )
{
	assert( NULL != chan );
	return ( chan->timeUntilMemoryChanged_ms == TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS ) ? true : false;
}
void TDIM_resetConditionToSaveNvm( tTdimChannelStruct *chan )
{
	assert( NULL != chan );
	chan->timeUntilMemoryChanged_ms = TIME_MEMORY_UP2DATE_MS;
}
void TDIM_processChannel( tTdimChannelStruct *chan, bool buttonPressed )
{
	processButton( chan, buttonPressed );
	
	switch( chan->state )
	{
		case STATE_INIT:
		{	
// 			if( chan->nvm.intensityPreset != INTENSITY_OFF )
// 			{
// 				#if (TDIM_CONF_POWERUP_DIMM_DIRECTION == TDIM_DIMM_DIR_DOWN)
// 				chan->dimmDirection = DIMM_DIR_DOWN;
// 				#elif (TDIM_CONF_POWERUP_DIMM_DIRECTION == TDIM_DIMM_DIR_UP)
// 				chan->dimmDirection = DIMM_DIR_UP;
// 				#endif
// 	
// 				setNewFadeLevel( chan );
// 				chan->state = STATE_POWER_CONTROL;
// 			}
// 			else
			{
				chan->state = STATE_IDLE;
			}
			break;
		}
		case STATE_IDLE:
		{
			switch( chan->buttonState )
			{
				case BTN_STATE_SHORT_PRESS_DETECTED:
				case BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED:
				{
					/* Powered off */
					if( chan->nvm.intensityPreset == INTENSITY_OFF )
					{
						chan->dimmDirection = DIMM_DIR_UP;
						chan->nvm.intensityPreset = chan->nvm.savedLevel;
					}
					else /* Powered on */
					{
						if( chan->nvm.mode == TDIM_MODE1_LAST_DIMM_VALUE )
						{
							chan->nvm.savedLevel = chan->nvm.intensityPreset;
						}
						chan->dimmDirection = DIMM_DIR_DOWN;
						chan->nvm.intensityPreset = INTENSITY_OFF;
					}
					
					setNewFadeLevel( chan );
					chan->state = STATE_POWER_CONTROL;
					chan->buttonState = BTN_STATE_RESET;
					break;
				}
				case BTN_STATE_LONG_PRESS_DETECTED:
				{
					if( chan->nvm.intensityPreset == INTENSITY_OFF )
					{
						chan->nvm.intensityPreset = TDIM_CONF_MIN_POWER_ON_DIMM_INTENSITY;
						setNewFadeLevel( chan );
					}
					chan->state = STATE_PROCESS_DIMMING;
					break;
				}
				case BTN_STATE_DOUBLE_PRESS_DETECTED:
				{
					chan->state = STATE_PROCESS_MODE;
					chan->buttonState = BTN_STATE_RESET;
					break;
				}
				default:
				break;
			}
			break;
		}
		case STATE_PROCESS_DIMMING:
		{
			if(chan->buttonState == BTN_STATE_LONG_PRESS_DETECTED
			|| chan->buttonState == BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED )
			{
				if( chan->dimmDirection == DIMM_DIR_UP )
				{
					if(    chan->timePressed % TDIM_CONF_DIMM_INTERVALL_MS == 0 
						&& chan->nvm.intensityPreset < INTENSITY_MAX     )
					{
						chan->nvm.intensityPreset++;
						setNewFadeLevel( chan );
					}
					fadeUp( chan );
				}
				else if( chan->dimmDirection == DIMM_DIR_DOWN )
				{
					if(    chan->timePressed % TDIM_CONF_DIMM_INTERVALL_MS == 0
						&& chan->nvm.intensityPreset != INTENSITY_MIN     )
					{
						chan->nvm.intensityPreset--;
						setNewFadeLevel( chan );
					}
					fadeDown( chan );
				}
					
					// intensity changed -> trigger 5min tout
					resetMemoryTimer( chan );
			
			}
			else
			{
				if( chan->dimmDirection == DIMM_DIR_UP )
				{
					if( true == fadeUp( chan ))
					{
						chan->dimmDirection = DIMM_DIR_DOWN;
						chan->state = STATE_IDLE;
					}
				}
				else
				{
					if( true == fadeDown( chan ))
					{
						chan->dimmDirection = DIMM_DIR_UP;
						chan->state = STATE_IDLE;
					}
				}
			}
			break;
		}
		case STATE_POWER_CONTROL:
		{
			bool finished = false;
			
			if( chan->dimmDirection == DIMM_DIR_UP )
			{
				if( true == fadeUp( chan ))
				{
					finished = true;
				}
			}
			else if( chan->dimmDirection == DIMM_DIR_DOWN )
			{
				if( true == fadeDown( chan ))
				{
					finished = true;
				}
			}
			if( finished )
			{
#if (TDIM_CONF_POWERUP_DIMM_DIRECTION == TDIM_DIMM_DIR_DOWN)
				chan->dimmDirection = ( chan->dimmDirection == DIMM_DIR_UP ) ? DIMM_DIR_DOWN : DIMM_DIR_UP;
#endif
				if( chan->buttonState == BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED )
				{
					chan->state = STATE_PROCESS_DIMMING;
				}
				else
				{
					chan->state = STATE_IDLE;
				}
				resetMemoryTimer( chan );
			}
			break;
		}
		case STATE_PROCESS_MODE:
		{
			if( chan->nvm.intensityPreset )
			{
				chan->nvm.mode = TDIM_MODE2_SAVED_DIMM_VALUE;
				chan->nvm.savedLevel = chan->nvm.intensityPreset;
			}
			else /* Powered off */
			{
				chan->nvm.mode = TDIM_MODE1_LAST_DIMM_VALUE;
			}
			// Set immediate save condition 
			chan->timeUntilMemoryChanged_ms = TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS;
			chan->state = STATE_IDLE;
			break;
		}
	}
	
	if( chan->timeUntilMemoryChanged_ms < TDIM_CONF_TIMEOUT_SAVE_NVM_CONDITION_MS )
	{
		chan->timeUntilMemoryChanged_ms++;
	}
	
	
	if( chan->state != STATE_IDLE 
	 && chan->state != STATE_PROCESS_MODE )
	{
		uint8_t outputLevel = chan->actualFadeLevel >> FADE_SHIFT_VALUE;
        
// 		uint8_t outputLevelFraction = chan->actualFadeLevel & FADE_FRACTION_MASK;
// 		
// 		uint16_t outputValue = chan->getLutValue(outputLevel);
//         
//         if(outputLevel != UINT8_MAX)
//         {   
// 	        outputValue += (( chan->getLutValue(outputLevel+1) - chan->getLutValue(outputLevel))
// 	                           * outputLevelFraction ) >> FADE_SHIFT_VALUE;
//         }                        
		
		chan->setPwmOutput( outputLevel );
	}
}


static inline void processButton( tTdimChannelStruct *chan, bool buttonPressed )
{	
	switch( chan->buttonState )
	{
		case BTN_STATE_RESET:
		{
			chan->timePressed = 0;
			chan->timeSecondPress = 0;
			chan->timeNotPressed = 0;
			chan->buttonState = BTN_STATE_IDLE;
			break;
		}
		case BTN_STATE_IDLE:
		{
			if( buttonPressed )
			{
				chan->timePressed++;
				if( chan->timePressed >= TIME_THRESHOLD_BETWEEN_SHORT_AND_LONG_PRESS )
				{
					chan->buttonState = BTN_STATE_LONG_PRESS_DETECTED;
				}
			}
			else
			{
				if( chan->timePressed < TIME_THRESHOLD_BETWEEN_SHORT_AND_LONG_PRESS
				&& chan->timePressed > TIME_THRESHOLD_DEBOUNCE_ON_SHORT_PRESS )
				{
					chan->timeNotPressed = 0;
					chan->buttonState = BTN_STATE_WAITING_FOR_DOUBLE_PRESS;
				}
				else
				{
					chan->timePressed = 0;
				}
			}
			break;
		}
		case BTN_STATE_WAITING_FOR_DOUBLE_PRESS:
		{
			if( buttonPressed )
			{
				chan->timeSecondPress++;
				if( chan->timeSecondPress > TIME_THRESHOLD_BETWEEN_SHORT_AND_LONG_PRESS )
				{
					chan->buttonState = BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED;
					chan->timeNotPressed = 0;
				}
			}
			else
			{
				if( chan->timeSecondPress > TIME_THRESHOLD_DEBOUNCE_ON_SHORT_PRESS
				&&	chan->timeSecondPress < TIME_THRESHOLD_BETWEEN_SHORT_AND_LONG_PRESS )
				{
					if( (chan->timeSecondPress + chan->timeNotPressed + chan->timePressed)
							<= TIME_THRESHOLD_DOUBLE_PRESS )
					{
						chan->buttonState = BTN_STATE_DOUBLE_PRESS_DETECTED;
					}
					else
					{
						chan->buttonState = BTN_STATE_SHORT_PRESS_DETECTED;
					}
				}
				else
				{
					chan->timeSecondPress = 0;
					
					chan->timeNotPressed++;
					if( chan->timeNotPressed + chan->timePressed
					>= TIME_THRESHOLD_DOUBLE_PRESS )
					{
						chan->buttonState = BTN_STATE_SHORT_PRESS_DETECTED;
					}
				}
			}
			break;
		}
		case BTN_STATE_LONG_PRESS_DETECTED:
		case BTN_STATE_SHORT_AND_LONG_PRESS_DETECTED:
		{
			if( buttonPressed )
			{
				chan->timePressed++;
			}
			else
			{
				chan->timeNotPressed++;
				if( chan->timeNotPressed >= TIME_THRESHOLD_DEBOUNCE_ON_SHORT_PRESS )
				{
					chan->buttonState = BTN_STATE_RESET;
				}
			}
			break;
		}
		case BTN_STATE_SHORT_PRESS_DETECTED:
		case BTN_STATE_DOUBLE_PRESS_DETECTED:
		{
			break;
		}
	}
}




static void setNewFadeLevel( tTdimChannelStruct *chan )
{
	uint_fast16_t shortActualLevel = chan->actualFadeLevel >> FADE_SHIFT_VALUE;
	uint_fast16_t shortNewLevel = chan->nvm.intensityPreset;
	
	chan->newFadeLevel = (uint_fast16_t)chan->nvm.intensityPreset << FADE_SHIFT_VALUE;
	
	if( shortNewLevel > shortActualLevel )
	{
		chan->fadeStep = (uint16_t)( shortNewLevel - shortActualLevel ) * FADE_STEP_SIZE;
		chan->newFadeLevel = shortNewLevel << FADE_SHIFT_VALUE;
	}
	else if( shortNewLevel < shortActualLevel )
	{
		chan->fadeStep = (uint16_t)(  shortActualLevel - shortNewLevel ) * FADE_STEP_SIZE;
		chan->newFadeLevel = shortNewLevel << FADE_SHIFT_VALUE;
	}
}


static bool fadeDown( tTdimChannelStruct *chan )
{
	bool fadeFinished = false;
	
	if( chan->actualFadeLevel > (chan->fadeStep + MIN_FADE_LEVEL_VALUE) )
	{
		chan->actualFadeLevel -= chan->fadeStep;
	}
	else
	{
		chan->actualFadeLevel = MIN_FADE_LEVEL_VALUE;
		fadeFinished = true;
	}
	if( chan->actualFadeLevel <= chan->newFadeLevel )
	{
		chan->actualFadeLevel = chan->newFadeLevel;
		fadeFinished = true;
	}
	
	return fadeFinished;
}

static bool fadeUp( tTdimChannelStruct *chan )
{
	bool fadeFinished = false;
	
	if( chan->actualFadeLevel < ( MAX_FADE_LEVEL_VALUE - chan->fadeStep ) )
	{
		chan->actualFadeLevel += chan->fadeStep;
	}
	else
	{
		chan->actualFadeLevel = MAX_FADE_LEVEL_VALUE;
		fadeFinished = true;
	}
	if( chan->actualFadeLevel >= chan->newFadeLevel )
	{
		chan->actualFadeLevel = chan->newFadeLevel;
		fadeFinished = true;
	}
	
	return fadeFinished;
}