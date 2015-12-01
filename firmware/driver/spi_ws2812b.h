bool ICACHE_FLASH_ATTR ws2812b_init(void);
static void ws2812b_send_zero(void);
static void ws2812b_send_one(void);
void ws2812b_send_rgb(uint8_t, uint8_t, uint8_t);
void ws2812b_set_pattern(uint8_t, uint8_t, uint16_t, uint16_t);

/*
 * Tells the PatternTimerHandler what to do.
 */
struct PatternCfg {
	uint8_t pattern; /* which hard-coded pattern */
	uint8_t repeat;	/* how many times to repeat this pattern per iteration */
	uint8_t cur;	/* where are we now */
	uint16_t ms_delay; /* timer duration */
	uint16_t stringlen; /* how many leds in the string */
};
extern struct PatternCfg pcfg;
