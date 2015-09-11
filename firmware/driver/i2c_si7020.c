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

bool
SI7020_Init()
{
	i2c_master_gpio_init(5, 2);	// hardcoded ickiness!
	if (i2c_master_writeBytes1(SI7020_ADDRESS, SI7020_RESET))
	{
		os_delay_us(10000);
		IS_ALREADY_INITED = true;
		return true;
	}
	os_printf("Failed to write to SI7020 ... \r\n");
	return false;
}

bool
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

bool
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

uint16_t
SI7020_GetTemperature()
{
	uint16_t temp;
	uint8_t datum[2];

	if (!IS_ALREADY_INITED)
	       SI7020_Init();

	if((SI7020_Read_Temperature((uint16_t *)datum)))
		temp = ((((datum[1]<<8)|datum[0])*175.72)/65536)-46.85;
	else
		temp = 0xffff;
	return temp;
}

uint16_t
SI7020_GetHumidity()
{
	uint16_t hum;
	uint8_t datum[2];

	if (!IS_ALREADY_INITED)
	       SI7020_Init();

	if((SI7020_Read_Humidity((uint16_t *)datum)))
		hum = ((((datum[1]<<8)|datum[0])*125.0)/65536)-6.0;
	else
		hum = 0xffff;
	return hum;
}
