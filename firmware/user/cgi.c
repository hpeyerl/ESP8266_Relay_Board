/*
Some random cgi routines.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "cgi.h"
#include "io.h"
#include <ip_addr.h>
#include "espmissingincludes.h"
#include "dht22.h"
#include "ds18b20.h"
#include "i2c_si7020.h"
#include "spi_max31855.h"
#include "spi_ws2812b.h"
#include "sntp.h"
#include "time_utils.h"
#include "config.h"

static const char *board_id_str[] = {
	"Relay Board",
	"Thermocouple Phrob",
	"Temperature Humidity Phrob" ,
	"Current Sensor Phrob",
	"Water Sensor Phrob",
	"Tilt Sensor Phrob",
	"Dual 16A Relay Phrob" ,
	"Single 16A Relay Phrob" ,
	"Signal Relay Phrob" ,
	"WS2812 LED Phrob",
};

int ICACHE_FLASH_ATTR tplIndex(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return HTTPD_CGI_DONE;

	os_sprintf(buff, "<nbsp>");

	if (os_strcmp(token, "board_id")==0) {
		os_sprintf(buff, "%s", board_id_str[sysCfg.board_id]);
	}
#ifdef CONFIG_MQTT
	if (os_strncmp(token, "config_mqtt", 10)==0) {
		os_strcpy(buff, "<li><a href=\"config/mqtt.tpl\">MQTT</a> settings.</li>");
	}
#endif
#if defined(CONFIG_DHT22) || defined(CONFIG_SI7020) || defined(CONFIG_MAX31855)
	if ((os_strcmp(token, "config_sensors")==0) && (
	    sysCfg.board_id == BOARD_ID_PHROB_TEMP_HUM ||
	    sysCfg.board_id == BOARD_ID_PHROB_THERMOCOUPLE ||
	    sysCfg.board_id == BOARD_ID_PHROB_HALL_EFFECT ||
	    sysCfg.board_id == BOARD_ID_PHROB_WATER)) {
		os_strcpy(buff, "<li>Sensor readings:</li><ul>");
	}
	if ((os_strcmp(token, "config_sensors_end")==0) && (
	    sysCfg.board_id == BOARD_ID_PHROB_TEMP_HUM ||
	    sysCfg.board_id == BOARD_ID_PHROB_THERMOCOUPLE ||
	    sysCfg.board_id == BOARD_ID_PHROB_HALL_EFFECT ||
	    sysCfg.board_id == BOARD_ID_PHROB_WATER)) {
		os_strcpy(buff, "</ul>");
	}
#endif
#ifdef CONFIG_DHT22
	if (os_strcmp(token, "config_dht22")==0 && sysCfg.board_id == BOARD_ID_RELAY_BOARD) {
		os_strcpy(buff, "<li>    <a href=\"control/dht22.tpl\">DHT22</a></li>");
	}
#endif
#ifdef CONFIG_SI7020
	if (os_strcmp(token, "config_si7020")==0 && sysCfg.board_id == BOARD_ID_PHROB_TEMP_HUM) {
		os_strcpy(buff, "<li>    <a href=\"control/si7020.tpl\">SI7020</a></li>");
	}
#endif
#ifdef CONFIG_MAX31855
	if ((os_strcmp(token, "config_max31855")==0) && (sysCfg.board_id == BOARD_ID_PHROB_THERMOCOUPLE)) {
		os_strcpy(buff, "<li>    <a href=\"control/max31855.tpl\">MAX31855</a>.</li>");
	}
#endif
#ifdef CONFIG_WS2812B
	if (os_strcmp(token, "control_ws2812")==0 && sysCfg.board_id == BOARD_ID_PHROB_WS2812B) {
		os_strcpy(buff, "<li>    <a href=\"control/ws2812b.tpl\">LEDs</a></li>");
	}
#endif
	if (os_strcmp(token, "config_relays")==0
		       	 && (sysCfg.board_id==BOARD_ID_PHROB_DUAL_RELAY
			 || sysCfg.board_id == BOARD_ID_PHROB_SINGLE_RELAY
			 || sysCfg.board_id == BOARD_ID_PHROB_SIGNAL_RELAY
			 || sysCfg.board_id == BOARD_ID_RELAY_BOARD))
	{
		os_strcpy(buff, "<li>Relay <a href=\"config/relay.tpl\">settings</a>.</li><li><a href=\"control/relay.html\">Relay</a> control page.</li>");
	}

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}



//Cgi that turns the Relays on or off according to the 'relayX' param in the GET data
int ICACHE_FLASH_ATTR cgiGPIO(HttpdConnData *connData) {
	int len;
	char buff[128];
	int gotcmd=0;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->getArgs, "relay1", buff, sizeof(buff));
	if (len>0) {
		currGPIO12State=atoi(buff);
		ioGPIO(currGPIO12State,relay1GPIO);
		gotcmd=1;
		//Manually switching relays means switching the thermostat off
		if(sysCfg.thermostat1state!=0) {
			sysCfg.thermostat1state=0;
		}
	}

	len=httpdFindArg(connData->getArgs, "relay2", buff, sizeof(buff));
	if (len>0) {
		currGPIO13State=atoi(buff);
		ioGPIO(currGPIO13State,relay2GPIO);
		gotcmd=1;
		//Manually switching relays means switching the thermostat off
		if(sysCfg.thermostat2state!=0) {
			sysCfg.thermostat2state=0;
		}

	}

	len=httpdFindArg(connData->getArgs, "relay3", buff, sizeof(buff));
	if (len>0) {
		currGPIO15State=atoi(buff);
		ioGPIO(currGPIO15State,relay3GPIO);
		gotcmd=1;
		//Manually switching relays means switching the thermostat off
		if(sysCfg.thermostat3state!=0) {
			sysCfg.thermostat3state=0;
		}
		
	}
	
	if(gotcmd==1) {
		if( sysCfg.relay_latching_enable) {		
			sysCfg.relay_1_state=currGPIO12State;					
			sysCfg.relay_2_state=currGPIO13State;
			sysCfg.relay_3_state=currGPIO15State;
			CFG_Save();
		}

		httpdRedirect(connData, "relay.tpl");
		return HTTPD_CGI_DONE;
	} else { //with no parameters returns JSON with relay state

		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "text/json");
		httpdHeader(connData, "Access-Control-Allow-Origin", "*");
		httpdEndHeaders(connData);

		len=os_sprintf(buff, "{\"relay1\": %d\n,\"relay1name\":\"%s\",\n\"relay2\": %d\n,\"relay2name\":\"%s\",\n\"relay3\": %d\n,\"relay3name\":\"%s\" }\n",  currGPIO12State,(char *)sysCfg.relay1name,currGPIO13State,(char *)sysCfg.relay2name,currGPIO15State,(char *)sysCfg.relay3name );
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;	
	}
}

void ICACHE_FLASH_ATTR tplGPIO(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;

	os_strcpy(buff, "Unknown");


	if (os_strcmp(token, "relay1name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay1name);
	}
	
	if (os_strcmp(token, "relay2name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay2name);
	}

	if (os_strcmp(token, "relay3name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay3name);
	}
	

	if (os_strcmp(token, "relay1")==0) {
		if (currGPIO12State) {
			os_strcpy(buff, "on");
		} else {
			os_strcpy(buff, "off");
		}
	os_printf("Relay 1 is now ");
	os_printf(buff);
	os_printf("\n ");

	}
	if (os_strcmp(token, "relay2")==0) {
		if (currGPIO13State) {
			os_strcpy(buff, "on");
		} else {
			os_strcpy(buff, "off");
		}
	os_printf("Relay 2 is now ");
	os_printf(buff);
	os_printf("\n ");


	}
	if (os_strcmp(token, "relay3")==0) {
		if (currGPIO15State) {
			os_strcpy(buff, "on");
		} else {
			os_strcpy(buff, "off");
		}
	os_printf("Relay 3 is now ");
	os_printf(buff);
	os_printf("\n ");

	}
	httpdSend(connData, buff, -1);
}


int ICACHE_FLASH_ATTR cgiws2812b(HttpdConnData *connData)
{
	int len;
	char buff[128];
	int gotcmd=0;
	int p=0;
	
	os_printf("here in ws2812b cgi [%s]\n", connData->post->buff);
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	len=httpdFindArg(connData->post->buff, "stringlen", buff, sizeof(buff));
	if (len>0) {
		ws2812b_set_stringlen(atoi(buff));
		gotcmd = 1;
	}
	len=httpdFindArg(connData->post->buff, "ms_delay", buff, sizeof(buff));
	if (len>0) {
		ws2812b_set_delay(atoi(buff));
		gotcmd = 1;
	}

	len=httpdFindArg(connData->post->buff, "brightness", buff, sizeof(buff));
	if (len>0) {
		ws2812b_set_brightness(atoi(buff));
		gotcmd = 1;
	}

	len=httpdFindArg(connData->post->buff, "pattern", buff, sizeof(buff));
	if (len>0) {
		gotcmd = 1;
		p = atoi(buff);
		if (p)
			ws2812b_set_pattern(p);
		else
			ws2812b_set_pattern(0);
	}
	
	if(gotcmd==1) {
		httpdRedirect(connData, "ws2812b.tpl");
		return HTTPD_CGI_DONE;
	} else {
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "text/json");
		httpdHeader(connData, "Access-Control-Allow-Origin", "*");
		httpdEndHeaders(connData);
		len=os_sprintf(buff, "ok");
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;	
	}
}


void ICACHE_FLASH_ATTR tplws2812b(HttpdConnData *connData, char *token, void **arg)
{
	char buff[512];

	if (token == NULL) return;

	os_sprintf(buff, "Invalid");
	
	if (os_strcmp(token, "cur_delay") == 0) {
		os_sprintf(buff, "%d", ws2812b_get_delay());
	}

	if (os_strcmp(token, "cur_brightness") == 0) {
		os_sprintf(buff, "%d", ws2812b_get_brightness());
	}

	if (os_strcmp(token, "cur_stringlen") == 0) {
		os_sprintf(buff, "%d", ws2812b_get_stringlen());
	}

	if (os_strcmp(token, "pattern_select") == 0) {
		os_sprintf(buff, 
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"1\" checked>\"All Off\"</td></tr>"
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"2\">\"All White\"</td></tr>"
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"3\">\"Flashing Primaries\"</td></tr>"
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"4\">\"Christmas\"</td></tr>"
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"5\">\"Fade Candy\"</td></tr>"
			"<tr><td><input type=\"radio\" name=\"pattern\" value=\"6\">\"Checkstop!\"</td></tr>");
	}

	httpdSend(connData, buff, -1);
}

static long hitCounter=0;

//Template code for the counter on the index page.
void ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;

	if (os_strcmp(token, "counter")==0) {
		hitCounter++;
		os_sprintf(buff, "%ld", hitCounter);
	}

	if (os_strcmp(token, "freeheap")==0) {
		os_sprintf(buff,"Free heap size:%d\n",system_get_free_heap_size());
  	}

	if (os_strcmp(token, "fwver")==0) {
		os_sprintf(buff,"%s",FWVER);
  	}

	httpdSend(connData, buff, -1);
}


//Cgi that reads the SPI flash. Assumes 512KByte flash.
int ICACHE_FLASH_ATTR cgiReadFlash(HttpdConnData *connData) {
	int *pos=(int *)&connData->cgiData;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if (*pos==0) {
		os_printf("Start flash download.\n");
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "application/bin");
		httpdEndHeaders(connData);
		*pos=0x40200000;
		return HTTPD_CGI_MORE;
	}
	//Send 1K of flash per call. We will get called again if we haven't sent 512K yet.
	espconn_sent(connData->conn, (uint8 *)(*pos), 1024);
	*pos+=1024;
	if (*pos>=0x40200000+(512*1024)) return HTTPD_CGI_DONE; else return HTTPD_CGI_MORE;
}


//Template code for the DHT 22 page.
void ICACHE_FLASH_ATTR tplDHT(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "temperature")==0) {
			dht_temp_str(buff);
	}
	if (os_strcmp(token, "humidity")==0) {
			dht_humi_str(buff);
	}	
	
	httpdSend(connData, buff, -1);
}



int ICACHE_FLASH_ATTR cgiDHT22(HttpdConnData *connData) {
	char buff[256];
	char temp[32];
	char humi[32];

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdHeader(connData, "Access-Control-Allow-Origin", "*");
	httpdEndHeaders(connData);

	dht_temp_str(temp);
	dht_humi_str(humi);
	
	os_sprintf(buff, "{ \n\"temperature\": \"%s\"\n , \n\"humidity\": \"%s\"\n}\n", temp, humi );
	
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}


//Template code for the DS18b20 page.
void ICACHE_FLASH_ATTR tplDS18b20 (HttpdConnData *connData, char *token, void **arg) {
	char buff[384];
	if (token==NULL) return;

	os_strcpy(buff, "Unknown");


	if (os_strcmp(token, "numds")==0) {	
		os_sprintf( buff,"%d", numds);
	}


	if (os_strcmp(token, "temperatures")==0) {	
	
		int Treading, SignBit, Whole, Fract;
		
		os_strcpy(buff, "");
	
		for(int i=0; i<numds; i++) {
		
			if(dsreading[i].success) {
			Treading = dsreading[i].temperature;
			   
			SignBit = Treading & 0x8000;  // test most sig bit
			if (SignBit) // negative
				Treading = (Treading ^ 0xffff) + 1; // 2's comp
	
			Whole = Treading >> 4;  // separate off the whole and fractional portions
			Fract = (Treading & 0xf) * 100 / 16;

			if (SignBit) // negative
				Whole*=-1;
		
			os_sprintf( buff+strlen(buff) ,"Sensor %d (%02x %02x %02x %02x %02x %02x %02x %02x) reading is %d.%d°C<br />", i+1, addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6], addr[i][7],
				Whole, Fract < 10 ? 0 : Fract);
			} else {
			os_sprintf( buff+strlen(buff) ,"Sensor %d (%02x %02x %02x %02x %02x %02x %02x %02x) reading is invalid<br />", i+1, addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], addr[i][6], addr[i][7]);
			}
		}

	}
	
	httpdSend(connData, buff, -1);

}



int ICACHE_FLASH_ATTR cgiDS18b20(HttpdConnData *connData) {
	char buff[256];
	char tmp[32];
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdHeader(connData, "Access-Control-Allow-Origin", "*");
	httpdEndHeaders(connData);

	ds_str(tmp,0);
	os_sprintf( buff, "{ \n\"temperature\": \"%s\"\n}\n",tmp );

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}



int ICACHE_FLASH_ATTR cgiState(HttpdConnData *connData) {
	char buff[512];
	char tmp[32];

	char temp[32];
	char humi[32];

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdHeader(connData, "Access-Control-Allow-Origin", "*");	
	httpdEndHeaders(connData);

	ds_str(tmp,0);

	dht_temp_str(temp);
	dht_humi_str(humi);
	
	os_sprintf( buff, "{ \n\"relay1\": \"%d\"\n,\n\"relay2\": \"%d\"\n,\n\"relay3\": \"%d\",\n  \n\"DHT22temperature\": \"%s\"\n , \n\"DHT22humidity\": \"%s\"\n,\"DS18B20temperature\": \"%s\"\n}\n",  currGPIO12State,currGPIO13State,currGPIO15State , temp,humi,tmp );

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}



int ICACHE_FLASH_ATTR cgiUI(HttpdConnData *connData) {

	char buff[128];

	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);

	os_strcpy(buff, "Unknown");
	httpdSend(connData, buff, -1);
	
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplUI(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "relay1name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay1name);
	}
	
	if (os_strcmp(token, "relay2name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay2name);
	}

	if (os_strcmp(token, "relay3name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay3name);
	}
	
	httpdSend(connData, buff, -1);


}

void ICACHE_FLASH_ATTR tplMQTT(HttpdConnData *connData, char *token, void **arg) {

	char buff[192];
	if (token==NULL) return;
	
	os_strcpy(buff, "");

	
	if (os_strcmp(token, "mqtt-enable")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.mqtt_enable);
	}

	if (os_strcmp(token, "mqtt-enabled")==0) {
			if (sysCfg.mqtt_enable) 
				os_sprintf(buff,"checked");
	}
	
	if (os_strcmp(token, "mqtt-use-ssl")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.mqtt_use_ssl);
	}

	if (os_strcmp(token, "mqtt-send-config")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.mqtt_send_config);
	}

	if (os_strcmp(token, "mqtt-send-config-enabled")==0) {
			if (sysCfg.mqtt_send_config)
				os_sprintf(buff,"checked");
	}

	if (os_strcmp(token, "mqtt-host")==0) {
			os_strcpy(buff, (char *)sysCfg.mqtt_host);
	}

	if (os_strcmp(token, "mqtt-port")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.mqtt_port);
	}

	if (os_strcmp(token, "mqtt-keepalive")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.mqtt_keepalive);
	}

	if (os_strcmp(token, "mqtt-deep-sleep-time")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.mqtt_deep_sleep_time);
	}

	if (os_strcmp(token, "mqtt-devid")==0) {
			os_strcpy(buff, (char *)sysCfg.mqtt_devid);
	}

	if (os_strcmp(token, "mqtt-user")==0) {
			os_strcpy(buff, (char *)sysCfg.mqtt_user);
	}

	if (os_strcmp(token, "mqtt-pass")==0) {
			os_strcpy(buff, (char *)sysCfg.mqtt_pass);
	}

	if (sysCfg.board_id==BOARD_ID_PHROB_DUAL_RELAY
		|| sysCfg.board_id == BOARD_ID_PHROB_SINGLE_RELAY
		|| sysCfg.board_id == BOARD_ID_PHROB_SIGNAL_RELAY
		|| sysCfg.board_id == BOARD_ID_RELAY_BOARD) {
		if (os_strcmp(token, "mqtt-relay-subs-topic")==0) {
			os_sprintf(buff, "<tr><td>Relays subs topic:</td><td><input type=\"text\" name=\"mqtt-relay-subs-topic\" id=\"mqtt-relay-subs-topic\" value=\"%s\" </td></tr>", sysCfg.mqtt_relay_subs_topic);
		}
	}
#if defined(CONFIG_WS2812B)
	if (sysCfg.board_id==BOARD_ID_PHROB_WS2812B) {
		if (os_strcmp(token, "mqtt-led-subs-topic")==0) {
			os_sprintf(buff, "<tr><td>LED subs topic:</td><td><input type=\"text\" name=\"mqtt-led-subs-topic\" id=\"mqtt-led-subs-topic\" value=\"%s\"</td></tr>", sysCfg.mqtt_led_subs_topic);
		}
	}
#endif

#if defined(CONFIG_SI7020) || defined(CONFIG_DHT22)
        if (sysCfg.board_id == BOARD_ID_PHROB_TEMP_HUM || sysCfg.board_id == BOARD_ID_RELAY_BOARD) {
		if (os_strcmp(token, "config_temphum1")==0) {
			os_sprintf(buff, "<tr><td>Temperature pub topic:</td><td><input type=\"text\" name=\"mqtt-temphum-temp-pub-topic\" id=\"mqtt-temphum-temp-pub-topic\" value=\"%s\">     </td></tr>", sysCfg.mqtt_temphum_temp_pub_topic);
		}
		if (os_strcmp(token, "config_temphum2")==0) {
			os_sprintf(buff, "<tr><td>Humidity pub topic:</td><td><input type=\"text\" name=\"mqtt-temphum-humi-pub-topic\" id=\"mqtt-temphum-humi-pub-topic\" value=\"%s\">     </td></tr>", sysCfg.mqtt_temphum_humi_pub_topic);
		}

	}
#endif

#if defined(CONFIG_DS18B20) || defined(CONFIG_MAX31855)
        if (sysCfg.board_id == BOARD_ID_PHROB_THERMOCOUPLE || sysCfg.board_id == BOARD_ID_RELAY_BOARD) {
		if (os_strcmp(token, "config_temp")==0) {
			os_sprintf(buff, "<tr><td>Temperature pub topic:</td><td><input type=\"text\" name=\"mqtt-temp-pub-topic\" id=\"mqtt-temp-pub-topic\" value=\"%s\">     </td></tr>", sysCfg.mqtt_temp_pub_topic);
		}
	}
#endif
	
	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiMQTT(HttpdConnData *connData) {
	char buff[128];
	int len;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->post->buff, "mqtt-enable", buff, sizeof(buff));
	sysCfg.mqtt_enable = (len > 0) ? 1:0;
	
	len=httpdFindArg(connData->post->buff, "mqtt-use-ssl", buff, sizeof(buff));
	sysCfg.mqtt_use_ssl = (len > 0) ? 1:0;
	
	len=httpdFindArg(connData->post->buff, "mqtt-send-config", buff, sizeof(buff));
	sysCfg.mqtt_send_config = (len > 0) ? 1:0;
	
	len=httpdFindArg(connData->post->buff, "mqtt-host", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_host,buff);
	}

	len=httpdFindArg(connData->post->buff, "mqtt-port", buff, sizeof(buff));
	if (len>0) {
		sysCfg.mqtt_port=atoi(buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-keepalive", buff, sizeof(buff));
	if (len>0) {
		sysCfg.mqtt_keepalive=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "mqtt-deep-sleep-time", buff, sizeof(buff));
	if (len>0) {
		sysCfg.mqtt_deep_sleep_time=atoi(buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-devid", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_devid,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-user", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_user,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-pass", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_pass,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-relay-subs-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_relay_subs_topic,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-temphum-temp-pub-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_temphum_temp_pub_topic,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-temphum-humi-pub-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_temphum_humi_pub_topic,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-devid", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_devid,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-user", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_user,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-pass", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_pass,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-temphum-temp-pub-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_temphum_temp_pub_topic,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "mqtt-temphum-humi-pub-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_temphum_humi_pub_topic,buff);
	}

	len=httpdFindArg(connData->post->buff, "mqtt-temp-pub-topic", buff, sizeof(buff));
	if (len>0) {
		os_sprintf((char *)sysCfg.mqtt_temp_pub_topic,buff);
	}

	CFG_Save();
		

	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}








void ICACHE_FLASH_ATTR tplHTTPD(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "httpd-auth")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.httpd_auth);
	}
	
	if (os_strcmp(token, "httpd-port")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.httpd_port);
	}

	if (os_strcmp(token, "httpd-user")==0) {
			os_strcpy(buff, (char *)sysCfg.httpd_user);
	}

	if (os_strcmp(token, "httpd-pass")==0) {
			os_strcpy(buff, (char *)sysCfg.httpd_pass);
	}

	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiHTTPD(HttpdConnData *connData) {
	char buff[128];
	int len;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}


	len=httpdFindArg(connData->post->buff, "httpd-auth", buff, sizeof(buff));
	sysCfg.httpd_auth = (len > 0) ? 1:0;
		
	len=httpdFindArg(connData->post->buff, "httpd-port", buff, sizeof(buff));
	if (len>0) {
		sysCfg.httpd_port=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "httpd-user", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.httpd_user,buff);
	}

	len=httpdFindArg(connData->post->buff, "httpd-pass", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.httpd_pass,buff);
	}
	

	CFG_Save();

	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}


void ICACHE_FLASH_ATTR tplNTP(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "ntp-enable")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.ntp_enable);
	}
	
	if (os_strcmp(token, "ntp-tz")==0) {
			os_sprintf(buff, "%d", (int)sysCfg.ntp_tz);
	}

	if (os_strcmp(token, "NTP")==0) {
		os_sprintf(buff,"Time: %s GMT%s%02d\n",epoch_to_str(sntp_time+(sntp_tz*3600)),sntp_tz > 0 ? "+" : "",sntp_tz);  
	}


	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiNTP(HttpdConnData *connData) {
	char buff[128];
	int len;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
	len=httpdFindArg(connData->post->buff, "ntp-enable", buff, sizeof(buff));
	sysCfg.ntp_enable = len > 0 ? 1:0;
	
	len=httpdFindArg(connData->post->buff, "ntp-tz", buff, sizeof(buff));
	if (len>0) {
		sysCfg.ntp_tz=atoi(buff);		
		sntp_tz=sysCfg.ntp_tz;
	}	
	
	CFG_Save();
	

	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR restartTimerCb(void *arg) {
	os_printf("Restarting..\n");
	system_restart();
		
}

int ICACHE_FLASH_ATTR cgiReset(HttpdConnData *connData)
{
	static ETSTimer restartTimer;
	//Schedule restart
	os_timer_disarm(&restartTimer);
	os_timer_setfn(&restartTimer, restartTimerCb, NULL);
	os_timer_arm(&restartTimer, 2000, 0);
	httpdRedirect(connData, "restarting.html");
	return HTTPD_CGI_DONE;
		
}



int ICACHE_FLASH_ATTR cgiRLYSettings(HttpdConnData *connData) {

	int len;
	char buff[128];

	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
	len=httpdFindArg(connData->post->buff, "relay-latching-enable", buff, sizeof(buff));
	sysCfg.relay_latching_enable = (len > 0) ? 1:0;
		

	len=httpdFindArg(connData->post->buff, "relay1name", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.relay1name,buff);
	}
	
	len=httpdFindArg(connData->post->buff, "relay2name", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.relay2name,buff);
	}

	len=httpdFindArg(connData->post->buff, "relay3name", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.relay3name,buff);
	}

	CFG_Save();
	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplRLYSettings(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "relay-latching-enable")==0) {
			os_strcpy(buff, sysCfg.relay_latching_enable == 1 ? "checked" : "" );
	}


	if (os_strcmp(token, "relay1name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay1name);
	}
	
	if (os_strcmp(token, "relay2name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay2name);
	}

	if (os_strcmp(token, "relay3name")==0) {
			os_strcpy(buff, (char *)sysCfg.relay3name);
	}
	
	httpdSend(connData, buff, -1);


}





int ICACHE_FLASH_ATTR cgiSensorSettings(HttpdConnData *connData) {

	int len;
	char buff[128];

	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
	len=httpdFindArg(connData->post->buff, "sensor-temp-enable", buff, sizeof(buff));
	sysCfg.sensor_temp_enable = (len > 0) ? 1:0;
		
	len=httpdFindArg(connData->post->buff, "sensor-temp-humi-enable", buff, sizeof(buff));
	sysCfg.sensor_temphum_enable = (len > 0) ? 1:0;


	len=httpdFindArg(connData->post->buff, "thermostat1-input", buff, sizeof(buff));
	if (len>0) {
		sysCfg.thermostat1_input=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "thermostat1-input", buff, sizeof(buff));
	if (len>0) {
		sysCfg.thermostat1_input=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "thermostat1hysteresishigh", buff, sizeof(buff));
	if (len>0) {
		sysCfg.thermostat1hysteresishigh=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "thermostat1hysteresislow", buff, sizeof(buff));
	if (len>0) {
		sysCfg.thermostat1hysteresislow=atoi(buff);
	}
	
	CFG_Save();
	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplSensorSettings(HttpdConnData *connData, char *token, void **arg) {

	char buff[128];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "sensor-ds18b20-enable")==0) {
			os_strcpy(buff, sysCfg.sensor_temp_enable == 1 ? "checked" : "" );
	}

	if (os_strcmp(token, "sensor-dht22-enable")==0) {
			os_strcpy(buff, sysCfg.sensor_temphum_enable == 1 ? "checked" : "" );
	}

	if (os_strcmp(token, "selectedds18b20")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 0 ? "selected" : "" );
	}

	if (os_strcmp(token, "selecteddht22t")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 1 ? "selected" : "" );
	}

	if (os_strcmp(token, "selecteddht22h")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 2 ? "selected" : "" );
	}

	if (os_strcmp(token, "selectedmqtt")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 3 ? "selected" : "" );
	}

	if (os_strcmp(token, "selectedserial")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 4 ? "selected" : "" );
	}

	if (os_strcmp(token, "selectedfixed")==0) {
			os_strcpy(buff, sysCfg.thermostat1_input == 5 ? "selected" : "" );
	}
	
	if (os_strcmp(token, "thermostat1hysteresishigh")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.thermostat1hysteresishigh);
	}

	if (os_strcmp(token, "thermostat1hysteresislow")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.thermostat1hysteresislow);
	}
	
	httpdSend(connData, buff, -1);


}




void ICACHE_FLASH_ATTR tplConfig(HttpdConnData *connData, char *token, void **arg) {
	char buff[255];
	if (token==NULL) return;

	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "board_id")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.board_id);
	}

	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiConfig(HttpdConnData *connData) {
	char buff[255];
	int len;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->getArgs, "js", buff, sizeof(buff));
	if (len>0) {

		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "text/javascript");
		httpdEndHeaders(connData);

		len=os_sprintf(buff, "var channel=%d;\n",(int)sysCfg.board_id );
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;	


	}

	len=httpdFindArg(connData->post->buff, "board_id", buff, sizeof(buff));
	os_printf("Set Board_ID to %s\n",buff);
	if (len>0) {
		sysCfg.board_id=atoi(buff);
	}

	CFG_Save();
	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR tplBroadcastD(HttpdConnData *connData, char *token, void **arg) {

	char buff[255];
	if (token==NULL) return;
	
	os_strcpy(buff, "Unknown");

	if (os_strcmp(token, "broadcastd-enable")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.broadcastd_enable);
	}
	
	if (os_strcmp(token, "broadcastd-port")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.broadcastd_port);
	}

	if (os_strcmp(token, "broadcastd-host")==0) {
			os_strcpy(buff, (char *)sysCfg.broadcastd_host);
	}

	if (os_strcmp(token, "broadcastd-URL")==0) {
			os_strcpy(buff, (char *)sysCfg.broadcastd_url);
	}

	if (os_strcmp(token, "broadcastd-thingspeak-channel")==0) {
			os_sprintf(buff,"%d", (int)sysCfg.broadcastd_thingspeak_channel);
	}

	if (os_strcmp(token, "broadcastd-ro-apikey")==0) {
			os_strcpy(buff, (char *)sysCfg.broadcastd_ro_apikey);
	}

	httpdSend(connData, buff, -1);
}

int ICACHE_FLASH_ATTR cgiBroadcastD(HttpdConnData *connData) {
	char buff[255];
	int len;
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->getArgs, "js", buff, sizeof(buff));
	if (len>0) {

		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "text/javascript");
		httpdEndHeaders(connData);

		len=os_sprintf(buff, "var channel=%d;\nvar ro_apikey=\"%s\";\n",(int)sysCfg.broadcastd_thingspeak_channel,(char *)sysCfg.broadcastd_ro_apikey );
		httpdSend(connData, buff, -1);
		return HTTPD_CGI_DONE;	


	}


	len=httpdFindArg(connData->post->buff, "broadcastd-enable", buff, sizeof(buff));
	sysCfg.broadcastd_enable = (len > 0) ? 1:0;
		
	len=httpdFindArg(connData->post->buff, "broadcastd-port", buff, sizeof(buff));
	if (len>0) {
		sysCfg.broadcastd_port=atoi(buff);
	}

	len=httpdFindArg(connData->post->buff, "broadcastd-host", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.broadcastd_host,buff);
	}

	len=httpdFindArg(connData->post->buff, "broadcastd-URL", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.broadcastd_url,buff);
	}

	len=httpdFindArg(connData->post->buff, "broadcastd-thingspeak-channel", buff, sizeof(buff));
	if (len>0) {
		sysCfg.broadcastd_thingspeak_channel=atoi(buff);
	}
	
	len=httpdFindArg(connData->post->buff, "broadcastd-ro-apikey", buff, sizeof(buff));
	if (len>0) {
		os_strcpy((char *)sysCfg.broadcastd_ro_apikey,buff);
	}

	CFG_Save();

	httpdRedirect(connData, "/");
	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR
cgiMax31855(HttpdConnData *connData)
{

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdRedirect(connData, "max31855.tpl");
	return HTTPD_CGI_DONE;

}
//
//Template code for the max31855 page.

int ICACHE_FLASH_ATTR tplMax31855(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	int16_t internal, kprobe;
	int ret;

	if (token==NULL) return HTTPD_CGI_DONE;

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "temps")==0) {
		ret = max31855_read_temps(&kprobe, &internal);
		if (ret == false)
			os_sprintf(buff, "Thermocouple error!");
		else
			os_sprintf(buff, "Internal: %d Kprobe: %d", internal, kprobe);
	}
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR
cgisi7020(HttpdConnData *connData)
{

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdRedirect(connData, "si7020.tpl");
	return HTTPD_CGI_DONE;

}
//
//Template code for the si7020 page.

int ICACHE_FLASH_ATTR tplsi7020(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	int16_t datum;

	if (token==NULL) return HTTPD_CGI_DONE;

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "temperature")==0) {
		datum = SI7020_GetTemperature();
		os_sprintf(buff, "%d", datum);
	}
	if (os_strcmp(token, "humidity")==0) {
		datum = SI7020_GetHumidity();
		os_sprintf(buff, "%d", datum);
	}

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}
#ifdef CONFIG_MLX91205
int ICACHE_FLASH_ATTR
cgimlx91205(HttpdConnData *connData)
{

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdRedirect(connData, "mlx91205.tpl");
	return HTTPD_CGI_DONE;

}
//
//Template code for the mlx91205 page.

int ICACHE_FLASH_ATTR tplmlx91205(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	int16_t datum;

	if (token==NULL) return HTTPD_CGI_DONE;

	os_strcpy(buff, "Unknown");
	if (os_strcmp(token, "current")==0) {
		datum = mlx91205_get();
		os_sprintf(buff, "%d", datum);
	}

	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}
#endif
