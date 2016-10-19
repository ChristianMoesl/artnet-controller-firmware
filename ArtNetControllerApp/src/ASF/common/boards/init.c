#include <asf.h>
#include <board.h>

#if BOARD == LED_CONTROLLER
#   include "led_controller/led_controller_init.h"
#elif BOARD == DMX_CONTROLLER
#   include "dmx_controller/dmx_controller_init.h"
#endif
