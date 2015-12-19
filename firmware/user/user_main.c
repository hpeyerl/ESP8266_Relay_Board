/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "espmissingincludes.h"
#include "ets_sys.h"
//#include "lwip/timers.h"
#include "user_interface.h"
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "cgithermostat.h"
#include "stdout.h"
#include "auth.h"
#include "sntp.h"
#include "time_utils.h"
#include "config.h"
#include "dht22.h"
#include "ds18b20.h"
#include "broadcastd.h"
#include "thermostat.h"
#include "wifi.h"
#include "mqtt.h"
#include "httpclient.h"
#include "captdns.h"
#include "spi_ws2812b.h"

//#include "netbios.h"
//#include "pwm.h"
//#include "cgipwm.h"

#ifdef CONFIG_OLED
#include "oled.h"
#endif

#ifdef CONFIG_MQTT
MQTT_Client mqttClient;
#endif

//Function that tells the authentication system what users/passwords live on the system.
//This is disabled in the default build; if you want to try it, enable the authBasic line in
//the builtInUrls below.
int ICACHE_FLASH_ATTR myPassFn(HttpdConnData *connData, int no, char *user, int userLen, char *pass, int passLen) {
	if (no==0) {
		os_strcpy(user, (char *)sysCfg.httpd_user);
		os_strcpy(pass, (char *)sysCfg.httpd_pass);
		return 1;
//Add more users this way. Check against incrementing no for each user added.
//	} else if (no==1) {
//		os_strcpy(user, "user1");
//		os_strcpy(pass, "something");
//		return 1;
	}
	return 0;
}


/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.tpl"},
	{"/index.tpl", cgiEspFsTemplate, tplIndex},
	{"/about.tpl", cgiEspFsTemplate, tplCounter},

	//{"/flash.bin", cgiReadFlash, NULL},

	{"/config/*", authBasic, myPassFn},
	{"/control/*", authBasic, myPassFn},

	{"/control/ui.tpl", cgiEspFsTemplate, tplUI},
	{"/control/relay.tpl", cgiEspFsTemplate, tplGPIO},
	{"/control/relay.cgi", cgiGPIO, NULL},
#ifdef CONFIG_WS2812B
	{"/control/ws2812b.tpl", cgiEspFsTemplate, tplws2812b},
	{"/control/ws2812b.cgi", cgiws2812b, NULL},
#endif
#ifdef CONFIG_DHT22
    {"/control/dht22.tpl", cgiEspFsTemplate, tplDHT},
    {"/control/dht22.cgi", cgiDHT22, NULL}, 
#endif
#ifdef CONFIG_DS18B20
    {"/control/ds18b20.tpl", cgiEspFsTemplate, tplDS18b20},
    {"/control/ds18b20.cgi", cgiDS18b20, NULL}, 
#endif
    {"/control/state.cgi", cgiState, NULL}, 
    {"/control/reset.cgi", cgiReset, NULL}, 
#ifdef CONFIG_THERMOSTAT
    {"/control/thermostat.tpl", cgiEspFsTemplate, tplThermostat},
    {"/control/thermostat.cgi", cgiThermostat, NULL}, 
#endif
#ifdef CONFIG_MAX31855
	{"/control/max31855.tpl", cgiEspFsTemplate, tplMax31855},
	{"/control/max31855.cgi", cgiMax31855, NULL},
#endif
#ifdef CONFIG_SI7020
	{"/control/si7020.tpl", cgiEspFsTemplate, tplsi7020},
	{"/control/si7020.cgi", cgisi7020, NULL},
#endif
#ifdef CONFIG_MLX91205
	{"/control/mlx91205.tpl", cgiEspFsTemplate, tplmlx91205},
	{"/control/mlx91205.cgi", cgimlx91205, NULL},
#endif
#ifdef CGIPWM_H
	{"/control/pwm.cgi", cgiPWM, NULL},
#endif
	{"/config/wifi", cgiRedirect, "/config/wifi/wifi.tpl"},
	{"/config/wifi/", cgiRedirect, "/config/wifi/wifi.tpl"},
	{"/config/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/config/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/config/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/config/wifi/setmode.cgi", cgiWiFiSetMode, NULL},
	{"/config/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
#ifdef CONFIG_MQTT
	{"/config/mqtt.tpl", cgiEspFsTemplate, tplMQTT},
	{"/config/mqtt.cgi", cgiMQTT, NULL},
#endif // CONFIG_MQTT
	{"/config/httpd.tpl", cgiEspFsTemplate, tplHTTPD},
	{"/config/httpd.cgi", cgiHTTPD, NULL},
	{"/config/broadcastd.tpl", cgiEspFsTemplate, tplBroadcastD},
	{"/config/broadcastd.cgi", cgiBroadcastD, NULL},
	{"/config/config.tpl", cgiEspFsTemplate, tplConfig},
	{"/config/config.cgi", cgiConfig, NULL},
	{"/config/ntp.tpl", cgiEspFsTemplate, tplNTP},
	{"/config/ntp.cgi", cgiNTP, NULL},
	{"/config/relay.tpl", cgiEspFsTemplate, tplRLYSettings},
	{"/config/relay.cgi", cgiRLYSettings, NULL},
	{"/config/sensor.tpl", cgiEspFsTemplate, tplSensorSettings},
	{"/config/sensor.cgi", cgiSensorSettings, NULL},

	
	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

#ifdef CONFIG_GET_EXTERNAL_IP

void ICACHE_FLASH_ATTR http_callback_IP(char * response, int http_status, char * full_response)
{
	//os_printf("http_status=%d\n", http_status);
	if (http_status != HTTP_STATUS_GENERIC_ERROR) {
		//os_printf("strlen(full_response)=%d\n", strlen(full_response));
		//os_printf("response=%s<EOF>\n", response);
		os_printf("External IP address=%s\n", response);
	}
}
#endif // CONFIG_GET_EXTERNAL_IP

#if 1
void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
#ifdef CONFIG_GET_EXTERNAL_IP
		os_printf("Trying to find external IP address\n");
		http_get("http://wtfismyip.com/text", http_callback_IP);
#endif

#ifdef CONFIG_MQTT
		if(sysCfg.mqtt_enable==1) {
			MQTT_Connect(&mqttClient);
		} else {
			MQTT_Disconnect(&mqttClient);
		}
#endif // CONFIG_MQTT
	}	
}
#endif


#ifdef CONFIG_MQTT
void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;

	 if (sysCfg.board_id==BOARD_ID_PHROB_DUAL_RELAY
		 || sysCfg.board_id == BOARD_ID_PHROB_SINGLE_RELAY
		 || sysCfg.board_id == BOARD_ID_PHROB_SIGNAL_RELAY
		 || sysCfg.board_id == BOARD_ID_RELAY_BOARD)
	{
		os_printf("MQTT: Connected.  Subscribing to: %s\r\n", sysCfg.mqtt_relay_subs_topic);
		MQTT_Subscribe(client, (char *)sysCfg.mqtt_relay_subs_topic,0);
	}

	if (sysCfg.board_id==BOARD_ID_PHROB_WS2812B)
	{
		os_printf("MQTT: Connected.  Subscribing to: %s\r\n", sysCfg.mqtt_led_subs_topic);
		MQTT_Subscribe(client, (char *)sysCfg.mqtt_led_subs_topic,0);
	}
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
//	MQTT_Client* client = (MQTT_Client*)args;
	os_printf("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t lengh)
{

	char *saveptr, *arg;
	char strTopic[topic_len + 1];
	char relayNum;
	int pulse=0;

	os_memcpy(strTopic, topic, topic_len);
	strTopic[topic_len] = '\0';

	char strData[lengh + 1];
	os_memcpy(strData, data, lengh);
	strData[lengh] = '\0';

	char strSubsTopic[strlen((char *)sysCfg.mqtt_relay_subs_topic)];
	os_strcpy(strSubsTopic,(char *)sysCfg.mqtt_relay_subs_topic);
	saveptr = strchr(strSubsTopic, '#');
	if (saveptr != NULL) {
		*saveptr = 0;
		saveptr++;
	}	// Now we have the Subscribed topic without args as a string, and args at saveptr

os_printf("strSubsTopic: %s strTopic: %s\n", strSubsTopic, strTopic);

	if (os_strncmp(strSubsTopic, strTopic, strlen(strSubsTopic)) != 0)
		return;

	// We've received a message for our subscribed topic.  Lets parse the arg(s).

	arg = &strTopic[strlen(strSubsTopic)];
	os_printf("arg: %s\n", arg);
	if ((saveptr = strchr(arg, '/')) != NULL) {
		// more than one arg.  Probably supposed to pulse it.
		*saveptr = 0;
		saveptr++;
		os_printf("saveptr: %s\n", saveptr);
		pulse=atoi(saveptr);
	}
	relayNum = arg[0];

	os_printf("Relay %d is now: %s \r\n", relayNum-'0', strData);
	
	if(relayNum=='1') {
		currGPIO12State=atoi(strData);
		ioGPIO(currGPIO12State,12);
		if (pulse) {
			os_delay_us(1000*pulse);
			currGPIO12State = (currGPIO12State == 1 ? 0 : 1);
			ioGPIO(currGPIO12State,12);
		}
	}

	if(relayNum=='2') {
		currGPIO13State=atoi(strData);
		ioGPIO(currGPIO13State,13);
		if (pulse) {
			os_delay_us(1000*pulse);
			currGPIO13State = (currGPIO13State == 1 ? 0 : 1);
			ioGPIO(currGPIO13State,13);
		}
	}

	if(relayNum=='3') {
		currGPIO15State=atoi(strData);
		ioGPIO(currGPIO15State,15);
		if (pulse) {
			os_delay_us(1000*pulse);
			currGPIO15State = (currGPIO15State == 1 ? 0 : 1);
			ioGPIO(currGPIO15State,15);
		}
	}
	
	if( sysCfg.relay_latching_enable) {		
		sysCfg.relay_1_state=currGPIO12State;					
		sysCfg.relay_2_state=currGPIO13State;
		sysCfg.relay_3_state=currGPIO15State;
		CFG_Save();
	}	
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;

    os_printf("MQTT: Published\r\n");

    if (sysCfg.mqtt_deep_sleep_time != 0)
		if(QUEUE_IsEmpty(&client->msgQueue) || client->sendTimeout != 0) {
			os_printf("Going to sleep for %d seconds ... zzzzzz\n", sysCfg.mqtt_deep_sleep_time);
			system_deep_sleep(sysCfg.mqtt_deep_sleep_time * 1000 * 1000);
		}
}

#endif  // CONFIG_MQTT

void ap_config()
{
	struct softap_config wiconfig;

	if (sysCfg.ap_bssid[0] != 0 && wifi_softap_get_config(&wiconfig)) {
		os_sprintf((char *)wiconfig.ssid, "%s", sysCfg.ap_bssid);
	}

}

//Main routine
void ICACHE_FLASH_ATTR user_init(void) {

	stdoutInit();	
	os_delay_us(100000);

	CFG_Load();
	// Should read a GPIO here or something.
	//wifi_set_opmode(0x2); //Force AP+STA mode
	ioInit();
	captdnsInit();
	WIFI_Connect(wifiConnectCb);

	httpdInit(builtInUrls, sysCfg.httpd_port);

	if(sysCfg.ntp_enable==1) {
		sntp_init(sysCfg.ntp_tz);	//timezone
	}
	
#ifdef CONFIG_MQTT
	if(sysCfg.mqtt_enable==1) {
		MQTT_InitConnection(&mqttClient, (uint8_t *)sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.mqtt_use_ssl );
		MQTT_InitClient(&mqttClient, (uint8_t *)sysCfg.mqtt_devid, (uint8_t *)sysCfg.mqtt_user, (uint8_t *)sysCfg.mqtt_pass, sysCfg.mqtt_keepalive,1);
		MQTT_OnConnected(&mqttClient, mqttConnectedCb);
		MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
		MQTT_OnPublished(&mqttClient, mqttPublishedCb);		
		MQTT_OnData(&mqttClient, mqttDataCb);
		
	}
#endif // CONFIG_MQTT
	
#ifdef CONFIG_DHT22
	if(sysCfg.sensor_temphum_enable && (
	    sysCfg.board_id == BOARD_ID_RELAY_BOARD || 
	    sysCfg.board_id == BOARD_ID_PHROB_DHT22) )
		DHTInit(SENSOR_DHT22, 30000);
#endif // CONFIG_DHT22
		
#ifdef CONFIG_DS18B20
	if (sysCfg.sensor_temp_enable &&
	    sysCfg.board_id == BOARD_ID_RELAY_BOARD )
		ds_init(30000);
#endif // CONFIG_DS18B20

#ifdef CONFIG_WS2812B
	if (sysCfg.board_id == BOARD_ID_PHROB_WS2812B)
		ws2812b_init();
#endif // CONFIG_WS2812B

	broadcastd_init();

#ifdef CONFIG_THERMOSTAT
	thermostat_init(30000);
#endif // CONFIG_THERMOSTAT

	// setup AP mode
	//ap_config();
#ifdef CONFIG_NETBIOS
	//Netbios to set the name
	struct softap_config wiconfig;
	os_memset(netbios_name, ' ', sizeof(netbios_name)-1);
	if(wifi_softap_get_config(&wiconfig)) {
		int i;
		for(i = 0; i < sizeof(netbios_name)-1; i++) {
			if(wiconfig.ssid[i] < ' ') break;
			netbios_name[i] = wiconfig.ssid[i];
		};
	}
	else os_sprintf(netbios_name, "ESP8266");
	netbios_name[sizeof(netbios_name)-1]='\0';
	netbios_init();
#endif // CONFIG_NETBIOS
		
	os_printf("\nRelay Board Ready\n");	
	os_printf("Free heap size:%d\n",system_get_free_heap_size());

	
#ifdef CGIPWM_H	
	//Mind the PWM pin!! defined in pwm.h
	duty=0;
	pwm_init( 50, &duty);
	pwm_set_duty(duty, 0);
    pwm_start();
#endif
	
#ifdef CONFIG_OLED
	OLEDInit();
#endif
	
}





