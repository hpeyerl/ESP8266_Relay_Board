#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "io.h"
#include "broadcastd.h"
#include "dht22.h"
#include "ds18b20.h"
#include "i2c_si7020.h"
#include "spi_max31855.h"
#include "config.h"
#include "mqtt.h"
#include "utils.h"
#include "httpclient.h"
#include "config.h"
  

/*
 * ----------------------------------------------------------------------------
 * "THE MODIFIED BEER-WARE LICENSE" (Revision 42):
 * Mathew Hall wrote this file. As long as you
 * retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day,
 * and you think this stuff is worth it, you can buy sprite_tm a beer in return.
 * ----------------------------------------------------------------------------
 */

#ifdef CONFIG_MQTT
MQTT_Client mqttClient;
static ETSTimer MQTTbroadcastTimer;
#endif
static ETSTimer broadcastTimer; 
 
//
// Broadcast to thingspeak
//
static void ICACHE_FLASH_ATTR broadcastReading(void *arg) {

	char buf[384];
	char buf2[255];
#if defined(CONFIG_DHT22) || defined(CONFIG_DS18B20)
	char t1[32];
	char t2[32];
	char t3[32];
#endif
	
	//double expand as sysCfg.broadcastd_url cntains placeholders as well
	os_sprintf(buf2,"http://%s:%d/%s",sysCfg.broadcastd_host,(int)sysCfg.broadcastd_port,sysCfg.broadcastd_url);
	
#ifdef CONFIG_DHT22
	if(sysCfg.sensor_temphum_enable)  {
		dht_temp_str(t2);
		dht_humi_str(t3);
		os_sprintf(buf,buf2,currGPIO12State,currGPIO13State,currGPIO15State,"N/A",t2,t3);
	}
#endif
	
#ifdef CONFIG_DS18B20
	if(sysCfg.sensor_ds18b20_enable)  { // If DS18b20 daemon is enabled, then send up to 3 sensor's data instead
		ds_str(t1,0);
		if(numds>1) ds_str(t2,1); //reuse to save space
		if(numds>2)  ds_str(t3,2); //reuse to save space
		os_sprintf(buf,buf2,currGPIO12State,currGPIO13State,currGPIO15State,t1,t2,t3);
	}
#endif
#ifdef CONFIG_SI7020
	if (sysCfg.board_id == BOARD_ID_PHROB_TEMP_HUM)
		os_sprintf(buf,buf2,SI7020_GetTemperature(),SI7020_GetHumidity());
#endif
#ifdef CONFIG_MAX31855
	if (sysCfg.board_id == BOARD_ID_PHROB_THERMOCOUPLE)
		os_sprintf(buf,buf2,"%d",max31855_read_ktemp());
#endif
		
	http_get(buf, http_callback_example);	
}
 

#ifdef CONFIG_MQTT
static ICACHE_FLASH_ATTR void MQTTbroadcastReading(void* arg){
	if(sysCfg.mqtt_enable==1) {
		//os_printf("Sending MQTT\n");
		
		if (mqttClient.connState != MQTT_DATA) {
			os_printf("MQTT: Waiting to publish....\n");
			return;		// Not ready to publish yet.
		}

		if(sysCfg.sensor_temphum_enable) {
#ifdef CONFIG_DHT22
			struct sensor_reading* result = readDHT();

			if(result->success) {
				char temp[32];
				char topic[128];
				int len;
				
				dht_temp_str(temp);
				len = os_strlen(temp);
				os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temphum_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
				
				dht_humi_str(temp);
				len = os_strlen(temp);
				os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temphum_humi_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
				go_to_sleep++;
			}
#endif
#ifdef CONFIG_SI7020
			char buf[32];
			char topic[128];
			int len;
			os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temphum_temp_pub_topic);
			len = os_sprintf(buf, "%d",SI7020_GetTemperature());
			MQTT_Publish(&mqttClient,topic,buf,len,0,0);
			os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temphum_humi_pub_topic);
			len = os_sprintf(buf, "%d",SI7020_GetHumidity());
			MQTT_Publish(&mqttClient,topic,buf,len,0,0);
			go_to_sleep++;
#endif
		}
		
		if(sysCfg.sensor_temp_enable) {
#ifdef CONFIG_DS18B20
			struct sensor_reading* result = read_ds18b20();
			if(result->success) {
				char temp[32];
				char topic[128];
				int len;
				ds_str(temp,0);
				len = os_strlen(temp);
				os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temp_pub_topic);
				MQTT_Publish(&mqttClient,topic,temp,len,0,0);
				os_printf("Published \"%s\" to topic \"%s\"\n",temp,topic);
				go_to_sleep++;
			}
#endif
#ifdef CONFIG_MAX31855
			char buf[32];
			char topic[128];
			int len;
			os_sprintf(topic,"%s/%s",sysCfg.mqtt_devid, sysCfg.mqtt_temp_pub_topic);
			len = os_sprintf(buf, "%d",max31855_read_ktemp());
			MQTT_Publish(&mqttClient,topic,buf,len,0,0);
			go_to_sleep++;
#endif
		}
    }
}

#endif // CONFIG_MQTT

uint8_t go_to_sleep;

void ICACHE_FLASH_ATTR broadcastd_init(void){

	go_to_sleep = 0;
	int timeout = 60000;

	if (sysCfg.mqtt_deep_sleep_time != 0)
		timeout=1000;	// If we're coming out of deepsleep, then we want to broadcast right away.
#ifdef CONFIG_MQTT
	if(sysCfg.mqtt_enable==1 && (sysCfg.sensor_temp_enable || sysCfg.sensor_temphum_enable)) {
		os_printf("Arming MQTT broadcast timer for %d seconds\n", timeout/1000);  	
		os_timer_setfn(&MQTTbroadcastTimer, MQTTbroadcastReading, NULL);
		os_timer_arm(&MQTTbroadcastTimer, timeout, 1);
	}
#endif // CONFIG_MQTT
	
	if(sysCfg.broadcastd_enable==1) {
		os_timer_setfn(&broadcastTimer, broadcastReading, NULL);
		os_printf("Arming HTTP broadcast timer\n");  	
		os_timer_arm(&MQTTbroadcastTimer, timeout, 1);
	}
}
