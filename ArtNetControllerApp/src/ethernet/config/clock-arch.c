#include "global-conf.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "asf.h"

#include <util/atomic.h>

#include "clock-arch.h"

#include "asf.h"

//Counted time
static clock_time_t clock_datetime = 0;

static void sysTickCallback( void )
{
	tc45_clear_overflow( &TCC4 );
	clock_datetime++;
} 


//Initialise the clock
void clock_init( void )
{
	irqflags_t irqFlags = cpu_irq_save();
	
	tc45_enable(&TCC4);
	tc45_set_overflow_interrupt_callback(&TCC4, sysTickCallback);
	tc45_set_wgm(&TCC4, TC45_WG_NORMAL);
	tc45_write_period(&TCC4, 32000 );
	tc45_set_overflow_interrupt_level(&TCC4, TC45_INT_LVL_LO);
	tc45_write_clock_source(&TCC4, TC45_CLKSEL_DIV1_gc);
	
	cpu_irq_restore(irqFlags);
}

//Return time
clock_time_t clock_time( void )
{
	clock_time_t time;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		time = clock_datetime;
	}
	return time;
}
