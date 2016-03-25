#ifndef __I2C_TSL2561_H
#define	__I2C_TSL2561_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#define TSL2561_RESET	(0xfe)

bool  TSL2561_Init(void);
uint16_t TSL2561_GetLux(void);
#endif
