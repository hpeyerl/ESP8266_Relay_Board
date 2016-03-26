
// From https://www.adafruit.com/datasheets/TSL2561.pdf
//****************************************************************************
//
//  Copyright 2004−2005 TAOS, Inc.
//
//  THIS CODE AND INFORMATION IS PROVIDED ”AS IS” WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//     Module Name:
//           lux.cpp
//
//****************************************************************************

// lux equation approximation without floating point calculations

//////////////////////////////////////////////////////////////////////////////
//     Routine:     unsigned int CalculateLux(unsigned int ch0, unsigned int ch0, int iType)
//
//     Description: Calculate the approximate illuminance (lux) given the raw
//                  channel values of the TSL2560. The equation if implemented
//                  as a piece−wise linear approximation.
//
//     Arguments:   unsigned int iGain − gain, where 0:1X, 1:16X
//                  unsigned int tInt − integration time, where 0:13.7mS, 1:100mS, 2:402mS,
//                   3:Manual
//                  unsigned int ch0 − raw channel value from channel 0 of TSL2560
//                  unsigned int ch1 − raw channel value from channel 1 of TSL2560
//                  unsigned int iType − package type (T or CS)
//
//     Return:      unsigned int − the approximate illuminance (lux)
//
//////////////////////////////////////////////////////////////////////////////

#include "c_types.h"
#include "ets_sys.h"
#include "taos_lux.h"

unsigned int ICACHE_FLASH_ATTR
CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1, int iType)
{
	//------------------------------------------------------------------------
	// first, scale the channel values depending on the gain and integration time
	// 16X, 402mS is nominal.
	// scale if integration time is NOT 402 msec
	unsigned long chScale;
	unsigned long channel1;
	unsigned long channel0;
	switch (tInt)
	{
		case 0:    // 13.7 msec
			chScale = CHSCALE_TINT0;
			break;
		case 1:    // 101 msec
			chScale = CHSCALE_TINT1;
			break;
		default:   // assume no scaling
			chScale = (1 << CH_SCALE);
			break;
	}
	// scale if gain is NOT 16X
	if (!iGain) chScale = chScale << 4;   // scale 1X to 16X
	// scale the channel values
	channel0 = (ch0 * chScale) >> CH_SCALE;
	channel1 = (ch1 * chScale) >> CH_SCALE;
	//------------------------------------------------------------------------
	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	unsigned long ratio1 = 0;
	if (channel0 != 0) ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
	// round the ratio value
	unsigned long ratio = (ratio1 + 1) >> 1;
	// is ratio <= eachBreak ?
	unsigned int b=0, m=0;
	switch (iType)
	{
		case 0: // T, FN and CL package
			if ((ratio >= 0) && (ratio <= K1T))
				{b=B1T; m=M1T;}
			else if (ratio <= K2T)
				{b=B2T; m=M2T;}
			else if (ratio <= K3T)
				{b=B3T; m=M3T;}
			else if (ratio <= K4T)
				{b=B4T; m=M4T;}
			else if (ratio <= K5T)
				{b=B5T; m=M5T;}
			else if (ratio <= K6T)
				{b=B6T; m=M6T;}
			else if (ratio <= K7T)
				{b=B7T; m=M7T;}
			else if (ratio > K8T)
				{b=B8T; m=M8T;}
			break;
#ifdef TAOS_2561_CS_PACKAGE
		case 1:// CS package
			if ((ratio >= 0) && (ratio <= K1C))
				{b=B1C; m=M1C;}
			else if (ratio <= K2C)
				{b=B2C; m=M2C;}
			else if (ratio <= K3C)
				{b=B3C; m=M3C;}
			else if (ratio <= K4C)
				{b=B4C; m=M4C;}
			else if (ratio <= K5C)
				{b=B5C; m=M5C;}
			else if (ratio <= K6C)
				{b=B6C; m=M6C;}
			else if (ratio <= K7C)
				{b=B7C; m=M7C;}
			else if (ratio > K8C)
				{b=B8C; m=M8C;}
			break;
#endif
	}
	unsigned long temp;
	temp = ((channel0 * b) - (channel1 * m));
	// do not allow negative lux value
	if (temp < 0) temp = 0;
	// round lsb (2^(LUX_SCALE-1))
	temp += (1 << (LUX_SCALE-1));
	// strip off fractional portion
	unsigned long lux = temp >> LUX_SCALE;
	return(lux);
}

