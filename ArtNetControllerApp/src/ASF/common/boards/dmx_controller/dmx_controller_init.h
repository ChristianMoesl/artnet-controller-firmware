#ifndef DMX_CONTROLLER_INIT_H_
#define DMX_CONTROLLER_INIT_H_

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
// 	osc_enable(OSC_ID_RC32MHZ);
// 	osc_enable(OSC_ID_RC32KHZ);
//  	osc_wait_ready(OSC_ID_RC32MHZ);
// 	osc_wait_ready(OSC_ID_RC32KHZ);
// 	osc_enable_autocalibration(OSC_ID_RC32MHZ, OSC_ID_RC32KHZ);
// 	sysclk_set_source(CLK_SCLKSEL_RC32M_gc);
	
	// Configure clock to 32MHz
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_RC32KEN_bm;  /* Enable the internal 32MHz & 32KHz oscillators */
	while(!(OSC.STATUS & OSC_RC32KRDY_bm));       /* Wait for 32Khz oscillator to stabilize */
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));       /* Wait for 32MHz oscillator to stabilize */
	DFLLRC32M.CTRL = DFLL_ENABLE_bm ;             /* Enable DFLL - defaults to calibrate against internal 32Khz clock */
	CCP = CCP_IOREG_gc;                           /* Disable register security for clock update */
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;              /* Switch to 32MHz clock */
	OSC.CTRL &= ~OSC_RC2MEN_bm;                   /* Disable 2Mhz oscillator */
	
	pmic_init();
	pmic_enable_level( PMIC_LVL_LOW );
	pmic_enable_level( PMIC_LVL_MEDIUM );
	pmic_enable_level( PMIC_LVL_HIGH );
    
	ioport_init();
	ioport_set_port_dir( IOPORT_PORTA, PORTA_DIR_INIT, IOPORT_DIR_OUTPUT );
	ioport_set_port_dir( IOPORT_PORTC, PORTC_DIR_INIT, IOPORT_DIR_OUTPUT );
	ioport_set_port_dir( IOPORT_PORTD, PORTD_DIR_INIT, IOPORT_DIR_OUTPUT );
	ioport_set_port_dir( IOPORT_PORTA, ~PORTA_DIR_INIT, IOPORT_DIR_INPUT );
	ioport_set_port_dir( IOPORT_PORTC, ~PORTC_DIR_INIT, IOPORT_DIR_INPUT );
	ioport_set_port_dir( IOPORT_PORTD, ~PORTD_DIR_INIT, IOPORT_DIR_INPUT );
	ioport_set_port_level( IOPORT_PORTA, PORTA_OUT_INIT, IOPORT_PIN_LEVEL_HIGH );
	ioport_set_port_level( IOPORT_PORTC, PORTC_OUT_INIT, IOPORT_PIN_LEVEL_HIGH );
	ioport_set_port_level( IOPORT_PORTD, PORTD_OUT_INIT, IOPORT_PIN_LEVEL_HIGH );
	ioport_set_port_level( IOPORT_PORTA, ~PORTA_OUT_INIT, IOPORT_PIN_LEVEL_LOW );
	ioport_set_port_level( IOPORT_PORTC, ~PORTC_OUT_INIT, IOPORT_PIN_LEVEL_LOW );
	ioport_set_port_level( IOPORT_PORTD, ~PORTD_OUT_INIT, IOPORT_PIN_LEVEL_LOW );
	ioport_set_port_mode( IOPORT_PORTA, PORTA_PULLUP_INIT, IOPORT_MODE_PULLUP);
	ioport_set_port_mode( IOPORT_PORTC, PORTC_PULLUP_INIT, IOPORT_MODE_PULLUP);
	ioport_set_port_mode( IOPORT_PORTD, PORTD_PULLUP_INIT, IOPORT_MODE_PULLUP);
}

#endif /* DMX_CONTROLLER_INIT_H_ */