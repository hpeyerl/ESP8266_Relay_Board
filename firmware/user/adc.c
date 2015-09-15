#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

//system_adc_read();

uint16_t ICACHE_FLASH_ATTR
mlx91205_get()
{
	return system_adc_read();
}
