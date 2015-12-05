#ifdef CONFIG_WS2812B
/*
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <ets_sys.h>
#include <gpio.h>
#include <mem.h>
#include <osapi.h>
#include <upgrade.h>
#include "espmissingincludes.h"
#include "config.h"

#include "osapi.h"

#include "spi_master.h"
#include "spi_register.h"

#include "spi_ws2812b.h"
void PatternTimerHandler(void *);

#define SPI_DEV 1	// HSPI
#define DELAYTIME 0
#define WS2812B_GPIO	13

// eww. globals.
//static uint8_t R[32] = { 0, 1, 3, 6, 7, 10, 12, 14, 18, 21, 25, 29, 33, 37, 43, 48, 54, 60, 67, 74, 82, 91, 100, 109, 118, 129, 140, 151, 162, 174, 187, 200};
//static uint8_t G[32] = { 0, 2, 4, 7, 9, 12, 14, 17, 21, 25, 29, 34, 39, 44, 51, 57, 64, 71, 79, 88, 97, 107, 118, 129, 140, 152, 165, 178, 192, 206, 221, 237};
//static uint8_t B[32] = { 0, 1, 3, 4, 6, 7, 9, 11, 14, 16, 19, 22, 25, 29, 33, 37, 42, 46, 52, 57, 64, 70, 77, 84, 92, 100, 108, 117, 126, 135, 145, 155};
//
static bool initialized = 0;
/*
 * Tells the PatternTimerHandler what to do.
 */
struct PatternCfg pcfg;

static ETSTimer PatternTimer;
static struct Pattern {
	int size;
	uint8_t rgb[24][3];
} patterns[6] = {
	{
		.size = 1,
		.rgb = {
			{ 0x00, 0x00, 0x00 }, },
	},
	{
		.size = 1,
		.rgb = {
			{ 0xff, 0xff, 0xff }, },
	},
	{
		.size = 8,
		.rgb = {
			{ 0xff, 0xff, 0xff },
			{ 0xff, 0xff, 0x00 },
			{ 0xff, 0x00, 0xff },
			{ 0x00, 0xff, 0xff },
			{ 0xff, 0x00, 0x00 },
			{ 0x00, 0xff, 0x00 },
			{ 0x00, 0x00, 0xff },
			{ 0x00, 0xff, 0x00 },
		},
	},
	{
		.size = 4,
		.rgb = {
			{ 0x78, 0, 0 },
			{ 0x78, 0, 0 },
			{ 0x0, 0x78, 0x0 },
			{ 0x0, 0x78, 0x0 },
		},
	},
	{
		.size = 22,
		.rgb = {
			{ 200, 237, 155},
			{ 200, 237, 155},
			{ 151, 178, 126 },
			{ 151, 178, 126 },
			{ 118, 140, 100 },
			{ 118, 140, 100 },
			{ 91, 107, 77 },
			{ 91, 107, 77 },
			{ 67, 79, 57 },
			{ 67, 79, 57 },
			{ 48, 57, 42 },
			{ 48, 57, 42 },
			{ 33, 39, 29 },
			{ 33, 39, 29 },
			{ 21, 25, 19 },
			{ 21, 25, 19 },
			{ 12, 14, 11 },
			{ 12, 14, 11 },
			{ 6, 7, 4 },
			{ 6, 7, 4 },
			{ 1, 2, 1 },
			{ 1, 2, 1 },
		},
	},
	{
		.size = 2,
		.rgb = {
			{ 0xff, 0x0, 0x0 },
			{ 0x0, 0x0, 0xff },
		},
	},
};



/*
 * For SPI, dev is our SS
 */
bool
ICACHE_FLASH_ATTR
ws2812b_init(void)
{
	if (initialized || sysCfg.board_id != BOARD_ID_PHROB_WS2812B)
		return true;
	spi_init_gpio(SPI_DEV, SPI_CLK_USE_DIV);
	spi_clock(SPI_DEV, SPI_CLK_PREDIV, SPI_CLK_CNTDIV);
	spi_tx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	spi_rx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	SET_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_CS_SETUP|SPI_CS_HOLD);
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_FLASH_MODE);
	initialized = 1;
	pcfg.stringlen = 18;
	pcfg.ms_delay = 500;
	pcfg.brightness = 8;
	os_timer_setfn(&PatternTimer, PatternTimerHandler, NULL);
	return true;
}



static void __attribute__((optimize("O2")))
// ICACHE_FLASH_ATTR
ws2812b_send_zero(void)
{

	int xtemp;
	xtemp = spi_transaction(1, 8, 0x80, 0, 0, 0, 0, 0, 0);
}

static void __attribute__((optimize("O2")))
// ICACHE_FLASH_ATTR
ws2812b_send_one(void)
{
	int xtemp;
	xtemp = spi_transaction(1, 8, 0xe0, 0, 0, 0, 0, 0, 0);
}

static inline void
ws2812b_send_color(uint8_t c)
{
	uint8_t bit=0x80;
	while(bit) {
		if (c&bit)
			ws2812b_send_one();
		else
			ws2812b_send_zero();
		bit>>=1;
	}
}

void
ws2812b_send_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	ws2812b_send_color(g);
	ws2812b_send_color(r);
	ws2812b_send_color(b);
}

void ICACHE_FLASH_ATTR
ws2812b_set_timer_delay()
{
	os_timer_disarm(&PatternTimer);
	os_timer_arm(&PatternTimer, pcfg.ms_delay, 1);
}

void
ICACHE_FLASH_ATTR
ws2812b_set_brightness(uint8_t brightness)
{
	pcfg.brightness = brightness;
}

uint8_t
ICACHE_FLASH_ATTR
ws2812b_get_brightness(void)
{
	return(pcfg.brightness);
}

void
ICACHE_FLASH_ATTR
ws2812b_set_stringlen(uint16_t stringlen)
{
	if (stringlen)
		pcfg.stringlen = stringlen;
	else
		pcfg.stringlen = 18; /* start with something */
}

uint16_t
ICACHE_FLASH_ATTR
ws2812b_get_stringlen(void)
{
	return(pcfg.stringlen);
}

void
ICACHE_FLASH_ATTR
ws2812b_set_delay(uint16_t delay)
{
	pcfg.ms_delay = delay;
	ws2812b_set_timer_delay();
}

uint16_t
ICACHE_FLASH_ATTR
ws2812b_get_delay(void)
{
	return(pcfg.ms_delay);
}

void
ICACHE_FLASH_ATTR
ws2812b_set_pattern(uint8_t pattern)
{
	if (!initialized)
		return;
	pcfg.pattern = pattern;
	pcfg.cur = 0;
	if (pattern)	// -1 turns off pattern
		ws2812b_set_timer_delay();
}

uint8_t
ICACHE_FLASH_ATTR
ws2812b_get_pattern(void)
{
	return(pcfg.brightness);
}

void __attribute__((optimize("O2")))
PatternTimerHandler(void *arg)
{
	int i, j, size;
	uint8_t r, g, b, p, x;

	p = pcfg.pattern-1;
	size = patterns[p].size;
	x = 8-pcfg.brightness;

	// Where in the pattern are we
	//                    How many RGB's to spit out
	//                                       next pattern element
	for (i=pcfg.cur, j=0; j<pcfg.stringlen;  i++, j++) {
		i %= size;
		r = patterns[p].rgb[i][0] >> x;
		g = patterns[p].rgb[i][1] >> x;
		b = patterns[p].rgb[i][2] >> x;
		ws2812b_send_rgb(r,g,b);
	}
	if (++pcfg.cur >= size)
		pcfg.cur = 0;

}

#endif
