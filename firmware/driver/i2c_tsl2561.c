#if defined(CONFIG_TSL2561)
#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#include "i2c_master.h"
#include "i2c_tsl2561.h"

#ifdef CONFIG_CMD_TSL2561_DEBUG
#define dbg(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

static bool IS_ALREADY_INITED = false;

bool ICACHE_FLASH_ATTR
TSL2561_Init()
{
#ifdef CONFIG_CMD_TSL2561_DEBUG
	uint8_t datum;
	uint16_t x;
	uint8_t offset;
	uint8_t len;
#endif
	os_printf("Initializing TSL2561..\n");
	i2c_master_gpio_init(5, 2);	// hardcoded ickiness!

	os_printf("Failed to write to TSL2561 ... \r\n");
	return false;
}

uint16_t ICACHE_FLASH_ATTR
TSL2561_GetLux()
{
	uint8_t datum[2];
	int lux = 0;
#if 0

	if (!IS_ALREADY_INITED) {
	       TSL2561_Init();
	       // First reading after reset seems bogus. 
	       // So lets wait a bit and do a throwaway read.
		os_delay_us(10000);
		TSL2561_Read_Temperature((uint16_t *)datum);
		os_delay_us(10000);
	}

	if((TSL2561_Read_Temperature((uint16_t *)datum)))
		/*
		 * Temperature seems to be off by 4-6(!) degrees on all of the sensors I've tested.
		 * Not sure why this is.  The on-chip heater is not on and it seems to be off the 
		 * same amount immediately after power on so it's not thermal coupling with the 
		 * CPU. This affects the humidity reading also.  So artifically adjust the temp
		 * here and humidity in TSL2561_GetHumidity.
		 */
		temp = ((((datum[1]<<8)|datum[0])*175.72)/65536)-46.85;	// should be -46.85
	else
		temp = 0xffff;
	os_printf("%s: datum is: 0x%x 0x%x  temp is 0x%x\n", __FUNCTION__, datum[1], datum[0], temp);
#endif
	return lux;
}

#endif
