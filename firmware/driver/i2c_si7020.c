#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#include "i2c_master.h"
#include "i2c_si7020.h"

#ifdef CONFIG_CMD_SI7020_DEBUG
#define dbg(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

static bool IS_ALREADY_INITED = false;

bool ICACHE_FLASH_ATTR
SI7020_Init()
{
	uint8_t datum;
	uint32_t *serial_num;
	i2c_master_gpio_init(5, 2);	// hardcoded ickiness!

	if (i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_RESET))
	{
		os_delay_us(15000);	// max time after issuing reset
		IS_ALREADY_INITED = true;
#ifdef CONFIG_CMD_SI7020_DEBUG
		i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_USER1);
		i2c_master_readUint8(SI7020_ADDRESS, 0, &datum);
		os_printf("%s: User 1 says: 0x%x %s\n", __FUNCTION__, datum, (datum&4?"Heater is on!":""));
		i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_USER1);
		datum &= ~4;	// turn off heater just in case it's on
		i2c_master_writeBytes1(SI7020_ADDRESS, datum);
#endif
		i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_SERIAL_A);
		i2c_master_readBytes(SI7020_ADDRESS, (uint8_t *)&serial_num, 4);
		os_printf("%s: Serial A is: %x", __FUNCTION__, serial_num);
		i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_SERIAL_B);
		i2c_master_readBytes(SI7020_ADDRESS, (uint8_t *)&serial_num, 4);
		os_printf("%s: Serial B is: %x", __FUNCTION__, serial_num);

		return true;
	}

	os_printf("Failed to write to SI7020 ... \r\n");
	return false;
}

bool ICACHE_FLASH_ATTR
SI7020_Read_Temperature(uint16_t *datum)
{
	if(!i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_TEMPERATURE)){
		return false;
	}

	datum[0] = 0;
	if(i2c_master_readUint16(SI7020_ADDRESS, 0, datum)){
		return true;
	}
	return false;
}

bool ICACHE_FLASH_ATTR
SI7020_Read_Humidity(uint16_t *datum)
{
	uint8 reg = SI7020_HUMIDITY;

	if(!i2c_master_writeBytes(SI7020_ADDRESS, &reg, 1)){
		return false;
	}

	datum[0] = 0;
	if(i2c_master_readUint16(SI7020_ADDRESS, 0, datum)){
		return true;
	}
	return false;
}

uint16_t ICACHE_FLASH_ATTR
SI7020_GetTemperature()
{
	uint16_t temp;
	uint8_t datum[2];

	if (!IS_ALREADY_INITED) {
	       SI7020_Init();
	       // First reading after reset seems bogus. 
	       // So lets wait a bit and do a throwaway read.
		os_delay_us(10000);
		SI7020_Read_Temperature((uint16_t *)datum);
		os_delay_us(10000);
	}

	if((SI7020_Read_Temperature((uint16_t *)datum)))
		/*
		 * Temperature seems to be off by 4-6(!) degrees on all of the sensors I've tested.
		 * Not sure why this is.  The on-chip heater is not on and it seems to be off the 
		 * same amount immediately after power on so it's not thermal coupling with the 
		 * CPU. This affects the humidity reading also.  So artifically adjust the temp
		 * here and humidity in SI7020_GetHumidity.
		 */
		temp = ((((datum[1]<<8)|datum[0])*175.72)/65536)-51.85;	// should be -46.85
	else
		temp = 0xffff;
	os_printf("%s: datum is: 0x%x 0x%x  temp is 0x%x\n", __FUNCTION__, datum[1], datum[0], temp);
	return temp;
}

uint16_t ICACHE_FLASH_ATTR
SI7020_GetHumidity()
{
	uint16_t hum;
	uint8_t datum[2];

	if (!IS_ALREADY_INITED) {
		SI7020_Init();
	       // First reading after reset seems bogus. 
	       // So lets wait a bit and do a throwaway read.
		os_delay_us(10000);
		SI7020_Read_Humidity((uint16_t *)datum);
		os_delay_us(10000);
	}

	/*
	 * See note in SI7020_GetTemperature
	 */
	if((SI7020_Read_Humidity((uint16_t *)datum)))
		hum = ((((datum[1]<<8)|datum[0])*125.0)/65536)-4.0; // Should be -6.0
	else
		hum = 0xffff;
	return hum;
}
