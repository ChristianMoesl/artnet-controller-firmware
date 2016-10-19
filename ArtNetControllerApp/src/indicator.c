/**
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */
#include "asf.h"
#include "indicator.h"
#include "timer.h"
#include "lighting.h"

#define TIME_IDLE_ON_PERIOD_MS  50
#define TIME_IDLE_OFF_PERIOD_MS 950

#define TIME_FLASH_X_TIMES_ON_PERIOD_MS 30
#define TIME_FLASH_X_TIMES_OFF_PERIOD_MS 50

static uint_fast8_t timesToFlashUpShot = 0;
static struct timer rxLedTimer, statusLedTimer;

void initIndicators(void)
{
    timer_set(&rxLedTimer, 0);
    timer_set(&statusLedTimer, 0);
}

void processIndicators(void)
{
    if (timer_expired(&statusLedTimer))
    {
        static bool isOn = false;
        
        timer_set(&statusLedTimer, isOn ? TIME_IDLE_OFF_PERIOD_MS : TIME_IDLE_ON_PERIOD_MS);
        isOn = !isOn;
        setStatusLed(isOn);
    }
    
    if(timer_expired(&rxLedTimer))
    {
       static bool isOn = false;
       
        if(isOn)
        {
            isOn = false;
            setReceiveLed(isOn);   
            timer_set(&rxLedTimer, TIME_FLASH_X_TIMES_OFF_PERIOD_MS);
        }
        else 
        {
            if(timesToFlashUpShot)
            {
                timesToFlashUpShot--;
                isOn = true;
                setReceiveLed(isOn);
                timer_set(&rxLedTimer, TIME_FLASH_X_TIMES_ON_PERIOD_MS);
            }    
            else
            {
                timer_set(&rxLedTimer, 0);
            }            
        }
    }
}

void toggleReceiveLed(void)
{
    if (timesToFlashUpShot < 3)
    {
        timesToFlashUpShot++;
    }
}
