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

void PatternTimerHandler(void *);

#define USE_SPI
#define SPI_DEV 1	// HSPI
#define DELAYTIME 0
#define WS2812B_GPIO	13

// eww. globals.
static bool initialized = 0;
static ETSTimer PatternTimer;
static int PatternTimerTimeout = 500;	// ms?
static struct Pattern {
	int size;
	uint8_t rgb[16][3];
} patterns[2] = {
	{
		.size = 1,
		.rgb = { { 0xff, 0xff, 0xff }, },
	},
	{
		.size = 8,
		.rgb = { { 0xff, 0xff, 0xff },
			{ 0xdd, 0xdd, 0xdd },
			{ 0xbb, 0xbb, 0xbb },
			{ 0x99, 0x99, 0x99 },
			{ 0x77, 0x77, 0x77 },
			{ 0x55, 0x55, 0x55 },
			{ 0x33, 0x33, 0x33 },
			{ 0x00, 0x00, 0x00 },
		},
	},
};


/*
 * Tells the PatternTimerHandler what to do.
 */
static struct PatternCfg {
	uint8_t pattern; /* which hard-coded pattern */
	uint8_t repeat;	/* how many times to repeat this pattern per iteration */
	uint16_t ms_delay; /* timer duration */
	uint8_t cur;	/* where are we right now */
} pcfg;

/*
 * For SPI, dev is our SS
 */
bool
// ICACHE_FLASH_ATTR
ws2812b_init(void)
{
	if (initialized || sysCfg.board_id != BOARD_ID_PHROB_WS2812B)
		return true;
#ifdef USE_SPI
	spi_init_gpio(SPI_DEV, SPI_CLK_USE_DIV);
	spi_clock(SPI_DEV, SPI_CLK_PREDIV, SPI_CLK_CNTDIV);
	spi_tx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	spi_rx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	SET_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_CS_SETUP|SPI_CS_HOLD);
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_FLASH_MODE);
#else
	// set GPIO13 as output.
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	initialized = 1;
#endif
	os_timer_setfn(&PatternTimer, PatternTimerHandler, NULL);
	os_timer_arm(&PatternTimer, PatternTimerTimeout, 1);
	return true;
}



static void __attribute__((optimize("O2")))
// ICACHE_FLASH_ATTR
ws2812b_send_zero(void)
{

#ifdef USE_SPI
	int xtemp;
	xtemp = spi_transaction(1, 8, 0x80, 0, 0, 0, 0, 0, 0);
	os_delay_us(DELAYTIME);
#else
	uint8_t i;
	i = 4; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << WS2812B_GPIO);
	i = 9; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << WS2812B_GPIO);
#endif
}

static void __attribute__((optimize("O2")))
// ICACHE_FLASH_ATTR
ws2812b_send_one(void)
{
#ifdef USE_SPI
	int xtemp;
	xtemp = spi_transaction(1, 8, 0xe0, 0, 0, 0, 0, 0, 0);
	os_delay_us(DELAYTIME);
#else
	uint8_t i;
	i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << WS2812B_GPIO);
	i = 6; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << WS2812B_GPIO);
#endif
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
	if (!initialized)
		return;
	ws2812b_send_color(g);
	ws2812b_send_color(r);
	ws2812b_send_color(b);
}

void
ws2812b_set_pattern(uint8_t pattern, uint8_t repeat, uint16_t ms_delay)
{
	pcfg.pattern = pattern;
	pcfg.repeat = repeat;
	pcfg.ms_delay = ms_delay;
}

void
PatternTimerHandler(void *arg)
{
	ws2812b_send_rgb(patterns[pcfg.pattern].rgb[pcfg.cur][0],patterns[pcfg.pattern].rgb[pcfg.cur][1],patterns[pcfg.pattern].rgb[pcfg.cur][2]);
}

#endif
