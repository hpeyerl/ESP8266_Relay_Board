bool ICACHE_FLASH_ATTR ws2812b_init(void);
static void ws2812b_send_zero(void);
static void ws2812b_send_one(void);
void ws2812b_send_rgb(uint8_t, uint8_t, uint8_t);
uint8_t ws2812b_get_pattern(void);
void ws2812b_set_pattern(uint8_t);
uint16_t ws2812b_get_delay(void);
void ws2812b_set_delay(uint16_t);
uint16_t ws2812b_get_stringlen(void);
void ws2812b_set_stringlen(uint16_t);
uint8_t ws2812b_get_brightness(void);
void ws2812b_set_brightness(uint8_t);
void ws2812b_save_pcfg();
void ws2812b_mqtt_pub_cb(uint32_t);

void ws2812b_set_timer_delay();

/*
 * Tells the PatternTimerHandler what to do.
 */
struct PatternCfg {
	uint8_t pattern; /* which hard-coded pattern */
	uint8_t cur;	/* where are we now */
	uint8_t brightness;	/* scalar for each RGB value */
	uint16_t ms_delay; /* timer duration */
	uint16_t stringlen; /* how many leds in the string */
};
