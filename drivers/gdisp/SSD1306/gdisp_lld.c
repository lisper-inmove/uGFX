/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    drivers/gdisp/SSD1306/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the SSD1306 display.
 */

#include "gfx.h"

#if GFX_USE_GDISP || defined(__DOXYGEN__)

#define GDISP_DRIVER_VMT			GDISPVMT_SSD1306
#include "../drivers/gdisp/SSD1306/gdisp_lld_config.h"
#include "gdisp/lld/gdisp_lld.h"

#include "board_SSD1306.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT		64		// This controller should support 32 (untested) or 64
#endif
#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH		128
#endif
#ifndef GDISP_INITIAL_CONTRAST
	#define GDISP_INITIAL_CONTRAST	100
#endif
#ifndef GDISP_INITIAL_BACKLIGHT
	#define GDISP_INITIAL_BACKLIGHT	100
#endif

#define GDISP_FLG_NEEDFLUSH			(GDISP_FLG_DRIVER<<0)

#include "SSD1306.h"

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

// Some common routines and macros
#define RAM(g)							((uint8_t *)g->priv)
#define write_cmd2(g, cmd1, cmd2)		{ write_cmd(g, cmd1); write_cmd(g, cmd2); }
#define write_cmd3(g, cmd1, cmd2, cmd3)	{ write_cmd(g, cmd1); write_cmd(g, cmd2); write_cmd(g, cmd3); }

// Some common routines and macros
#define delay(us)					gfxSleepMicroseconds(us)
#define delayms(ms)					gfxSleepMilliseconds(ms)

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * As this controller can't update on a pixel boundary we need to maintain the
 * the entire display surface in memory so that we can do the necessary bit
 * operations. Fortunately it is a small display in monochrome.
 * 64 * 128 / 8 = 1024 bytes.
 */

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	// The private area is the display surface.
	g->priv = gfxAlloc(GDISP_SCREEN_HEIGHT * GDISP_SCREEN_WIDTH / 8);

	// Initialise the board interface
	init_board(g);

	// Hardware reset
	setpin_reset(g, TRUE);
	gfxSleepMilliseconds(20);
	setpin_reset(g, FALSE);
	gfxSleepMilliseconds(20);

	acquire_bus(g);

	write_cmd(g, SSD1306_DISPLAYOFF);
	write_cmd2(g, SSD1306_SETDISPLAYCLOCKDIV, 0x80);
	write_cmd2(g, SSD1306_SETMULTIPLEX, GDISP_SCREEN_HEIGHT-1);
	write_cmd2(g, SSD1306_SETPRECHARGE, 0x1F);
	write_cmd2(g, SSD1306_SETDISPLAYOFFSET, 0);
	write_cmd(g, SSD1306_SETSTARTLINE | 0);
	write_cmd2(g, SSD1306_ENABLE_CHARGE_PUMP, 0x14);
	write_cmd2(g, SSD1306_MEMORYMODE, 0);
	write_cmd(g, SSD1306_SEGREMAP+1);
	write_cmd(g, SSD1306_COMSCANDEC);
	#if GDISP_SCREEN_HEIGHT == 64
		write_cmd2(g, SSD1306_SETCOMPINS, 0x12);
	#else
		write_cmd2(g, SSD1306_SETCOMPINS, 0x22);
	#endif
	write_cmd2(g, SSD1306_SETCONTRAST, (uint8_t)(GDISP_INITIAL_CONTRAST*256/101));	// Set initial contrast.
	write_cmd2(g, SSD1306_SETVCOMDETECT, 0x10);
	write_cmd(g, SSD1306_DISPLAYON);
	write_cmd(g, SSD1306_NORMALDISPLAY);
	write_cmd3(g, SSD1306_HV_COLUMN_ADDRESS, 0, GDISP_SCREEN_WIDTH-1);
	write_cmd3(g, SSD1306_HV_PAGE_ADDRESS, 0, GDISP_SCREEN_HEIGHT/8-1);

    // Finish Init
    post_init_board(g);

 	// Release the bus
	release_bus(g);

	/* Initialise the GDISP structure */
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
	g->g.Contrast = GDISP_INITIAL_CONTRAST;
	return TRUE;
}

#if GDISP_HARDWARE_FLUSH
	LLDSPEC void gdisp_lld_flush(GDisplay *g) {
		unsigned	i;

		// Don't flush if we don't need it.
		if (!(g->flags & GDISP_FLG_NEEDFLUSH))
			return;

		write_cmd(g, SSD1306_SETSTARTLINE | 0);

		for(i=0; i < GDISP_SCREEN_WIDTH * GDISP_SCREEN_HEIGHT/8; i+=GDISP_BUS_MAX_TRANSFER_SIZE)
			write_data(g, RAM(g)+i, GDISP_BUS_MAX_TRANSFER_SIZE);
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
		if (g->p.color != Black)
			RAM(g)[g->p.x + (g->p.y>>3)*GDISP_SCREEN_WIDTH] |=  (1<<(g->p.y&7));
		else
			RAM(g)[g->p.x + (g->p.y>>3)*GDISP_SCREEN_WIDTH] &=  ~(1<<(g->p.y&7));
		g->flags |= GDISP_FLG_NEEDFLUSH;
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC color_t gdisp_lld_get_pixel_color(GDisplay *g) {
		return (RAM(g)[g->p.x + (g->p.y>>3)*GDISP_SCREEN_WIDTH] & (1<<(g->p.y&7))) ? White : Black;
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_POWER:
			if (g->g.Powermode == (powermode_t)g->p.ptr)
				return;
			switch((powermode_t)g->p.ptr) {
			case powerOff:
			case powerSleep:
			case powerDeepSleep:
				acquire_bus(g);
				write_cmd(g, SSD1306_DISPLAYOFF);
				release_bus(g);
				break;
			case powerOn:
				acquire_bus(g);
				write_cmd(g, SSD1306_DISPLAYON);
				release_bus(g);
			default:
				return;
			}
			g->g.Powermode = (powermode_t)g->p.ptr;
			return;

		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
			case GDISP_ROTATE_0:
				acquire_bus(g);
				write_cmd(g, SSD1306_COMSCANDEC);
				write_cmd(g, SSD1306_SEGREMAP+1);
				GDISP.Height = GDISP_SCREEN_HEIGHT;
				GDISP.Width = GDISP_SCREEN_WIDTH;
				release_bus(g);
				break;
			case GDISP_ROTATE_180:
				acquire_bus(g);
				write_cmd(g, SSD1306_COMSCANINC);
				write_cmd(g, SSD1306_SEGREMAP);
				GDISP.Height = GDISP_SCREEN_HEIGHT;
				GDISP.Width = GDISP_SCREEN_WIDTH;
				release_bus(g);
				break;
			default:
				return;
			}
			g->g.Orientation = (orientation_t)value;
			return;

		case GDISP_CONTROL_CONTRAST:
            if ((unsigned)g->p.ptr > 100)
            	g->p.ptr = (void *)100;
			acquire_bus(g);
			write_cmd2(g, SSD1306_SETCONTRAST, (((uint16_t)value)<<8)/101);
			release_bus(g);
            g->g.Contrast = (unsigned)g->p.ptr;
			return;

		// Our own special controller code to inverse the display
		// 0 = normal, 1 = inverse
		case GDISP_CONTROL_INVERT:
			acquire_bus(g);
			write_cmd(g, g->p.ptr ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
			release_bus(g);
			return;
		}
	}
#endif // GDISP_NEED_CONTROL

#endif // GFX_USE_GDISP

