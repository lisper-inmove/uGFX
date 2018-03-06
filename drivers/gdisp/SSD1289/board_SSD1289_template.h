/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

static GFXINLINE void init_board(GDisplay *g) {
	(void) g;
}

static GFXINLINE void post_init_board(GDisplay *g) {
	(void) g;
}

static GFXINLINE void setpin_reset(GDisplay *g, bool_t state) {
	(void) g;
	(void) state;
}

static GFXINLINE void set_backlight(GDisplay *g, uint8_t percent) {
	(void) g;
	(void) percent;
}

static GFXINLINE void acquire_bus(GDisplay *g) {
	(void) g;
}

static GFXINLINE void release_bus(GDisplay *g) {
	(void) g;
}

static GFXINLINE void write_index(GDisplay *g, uint16_t index) {
	(void) g;
	(void) index;
}

static GFXINLINE void write_data(GDisplay *g, uint16_t data) {
	(void) g;
	(void) data;
}

static GFXINLINE void setreadmode(GDisplay *g) {
	(void) g;
}

static GFXINLINE void setwritemode(GDisplay *g) {
	(void) g;
}

static GFXINLINE uint16_t read_data(GDisplay *g) {
	(void) g;
	return 0;
}

//Optional define if your board interface supports it
//#define GDISP_USE_DMA			GFXON

// Optional define - valid only when GDISP_USE_DMA is GFXON
//#define GDISP_NO_DMA_FROM_STACK	GFXOFF

#if defined(GDISP_USE_DMA) && GDISP_USE_DMA

	static GFXINLINE void dma_with_noinc(GDisplay *g, color_t *buffer, int area) {
		(void) g;
		(void) buffer;
		(void) area;
	}

	static GFXINLINE void dma_with_inc(GDisplay *g, color_t *buffer, int area) {
		(void) g;
		(void) buffer;
		(void) area;
	}
#endif

#endif /* _GDISP_LLD_BOARD_H */
