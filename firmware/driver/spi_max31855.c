/*
 * Cold-Junction Compensated Thermocouple-to-Digital Converter
 * http://www.maximintegrated.com/datasheet/index.mvp/id/7273
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

#include "osapi.h"

#include "spi_master.h"
#include "spi_register.h"

#define SPI_DEV 1	// HSPI
static bool max31855_initialized = 0;
/*
 * For SPI, dev is our SS
 */
bool ICACHE_FLASH_ATTR
max31855_init()
{
	spi_init_gpio(SPI_DEV, SPI_CLK_USE_DIV);
	spi_clock(SPI_DEV, 4, 4); //5MHz
	spi_tx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	spi_rx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	SET_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_CS_SETUP|SPI_CS_HOLD);
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_FLASH_MODE);

	max31855_initialized = 1;
	return true;
}

#define TC_FAULT_MASK	0x1	// One of SCV, SCG, or OC faults detected
#define SCV_FAULT_MASK	0x4	// TC shorted to Vcc
#define SCG_FAULT_MASK  0x2	// TC shorted to gnd
#define OC_FAULT_MASK   0x1	// TC is open.

/*
 * Read either the kprobe or internal temperature
 */
bool ICACHE_FLASH_ATTR
max31855_read_temps(int16_t *kprobe, int16_t *internal)
{
	int16_t itemp=0;
	int16_t ktemp=0;
	int16_t xtemp;

#ifdef AT1284P

	// assert SS (active low)
	spi_ss(dev, 0);	// opt is our port.
	// Get k-probe temp
	xtemp = spi_xfer(0);	// [31:24]
	xtemp <<=8;
	xtemp |= spi_xfer(0);	// [23:16]
	//
	itemp |= spi_xfer(0);
	itemp <<=8;
	itemp |= spi_xfer(0);
	// release SS
	spi_ss(dev, 1);
#else
	uint32_t data;

	if (max31855_initialized == 0)
		max31855_init();

#if 0		// TODO: Find mode.
	uint32_t datum[5], votes[5];
	int i=0, j=0, k=0;
	// spi(1), cmd_bits(0), cmd(0), addr_bits(0), addr(0), dout_bits(0), dout(0), din_bits(32), dummy_bits(0)
	// collect 5 samples.
	while(i++<5) {
		datum[i] = spi_transaction(1, 0, 0, 0, 0, 0, 0, 32, 0);
		os_printf("Received from SPI: %x\r\n", datum);
		os_delay_us(10000);
	}
	i=0; j=0;
	// ignore spurious samples
	votes[j]++;	// vote for the first one
	while(i<5) {
		for (k=0;k<i;k++)
			if (datum[i] == datum[k])
	}
#else	
	data = spi_transaction(1, 0, 0, 0, 0, 0, 0, 32, 0);
#endif
	xtemp = ((data >>16)&0xffff);
	itemp = ((data & 0xffff));
	os_printf("xtemp is: 0x%x\r\nitemp is: 0x%x\r\n", xtemp, itemp);
#endif

	/*
	 * Check for faults.
	 */
	if (xtemp & TC_FAULT_MASK) {
		os_printf( "TC_FAULT\n");
		return false;
	}

	if (itemp & (SCV_FAULT_MASK|SCG_FAULT_MASK|OC_FAULT_MASK)) {
		os_printf( "Fault is: %x\n", itemp&(SCV_FAULT_MASK|SCG_FAULT_MASK|OC_FAULT_MASK));
		return false;
	}

	/*
	 * Convert TC temperature into degC
	 */
	xtemp >>= 2;
	ktemp = (xtemp & 0x1fff);	// 14 bit TC temp.
	if (xtemp&0x2000) {	// < 0 degC?
		ktemp = ~itemp;	// yes, 2's complement
		ktemp = data & 0x1fff;	// mask off any garbage we picked up.
		ktemp++;
		ktemp *= -1;
	}
	os_printf("ktemp*4 is %d\r\n", ktemp);
	ktemp /= 4;	// convert to degree celsius
	*kprobe = ktemp;
	/*
	 * Convert internal temperature into degC
	 */
	itemp &= 0xfff8;	// [15:4] contain a 12 bit internal temp.
	itemp = (itemp>>4)&0xfff;
	if (itemp & 0x800) {	// < 0 degC?
		itemp = ~itemp&0x7ff;	// 2's complement
		itemp += 1;
		itemp *= -1;
	}
	else
	{
		itemp &= 0x7ff;
	}
	os_printf("itemp*16 is %d\r\n", itemp);
	itemp /= 16;
	*internal = itemp;
	return true;
}

uint16_t ICACHE_FLASH_ATTR
max31855_read_itemp()
{
	int16_t kprobe, internal;
	bool ret;

	ret = max31855_read_temps(&kprobe, &internal);
	if (ret == true)
		return internal;
	else
		return 0xffff;	// MAX31855 reported an error.
}

uint16_t ICACHE_FLASH_ATTR
max31855_read_ktemp()
{
	int16_t kprobe, internal;
	bool ret;

	ret = max31855_read_temps(&kprobe, &internal);
	if (ret == true)
		return kprobe;
	else
		return 0xffff;
}

#ifdef CONFIG_ENABLE_MQTT
#include "lib/mqtt.h"

/*
 * Is this a reasonable limit?
 */
#define TOPIC_LEN 128

static int ICACHE_FLASH_ATTR
do_max31855_pub_kprobe(int argc, const char* const* argv)
{
	MQTT_Client *client = mqttGetConnectedClient();
	char buf[6];
	int buflen;
	char topic[TOPIC_LEN];

	if (client == NULL) {
		os_printf("MQTT Client not bound to broker\r\n");
		return -1;
	}

	os_sprintf(topic, "%s/max31855/kprobe/0", client->connect_info.client_id);
	buflen = os_sprintf(buf, "%d", max31855_read_ktemp());
	MQTT_Publish(client, topic, buf, buflen, 0, 0);
	return 0;
}

static int  ICACHE_FLASH_ATTR
do_max31855_pub_internal(int argc, const char* const* argv)
{
	MQTT_Client *client = mqttGetConnectedClient();
	char buf[6];
	int buflen;
	char topic[TOPIC_LEN];

	if (client == NULL) {
		os_printf("MQTT Client not bound to broker\r\n");
		return -1;
	}

	os_sprintf(topic, "%s/max31855/internal/0", client->connect_info.client_id);
	buflen = os_sprintf(buf, "%d", max31855_read_itemp());
	MQTT_Publish(client, topic, buf, buflen, 0, 0);
	return 0;
}

static int do_max31855_temps(int argc, const char* const* argv)
{
	int16_t kprobe, internal;

	max31855_read_temps(&kprobe, &internal);
	os_printf("Internal: %d\r\n Kprobe: %d\r\n", internal, kprobe);
	return 0;
}
#endif // MQTT
