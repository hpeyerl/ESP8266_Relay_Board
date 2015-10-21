#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int cgiGPIO(HttpdConnData *connData);
void tplGPIO(HttpdConnData *connData, char *token, void **arg);
int cgiReadFlash(HttpdConnData *connData);
void tplCounter(HttpdConnData *connData, char *token, void **arg);
void tplDHT(HttpdConnData *connData, char *token, void **arg);
int cgiDHT22(HttpdConnData *connData);
void tplDS18b20(HttpdConnData *connData, char *token, void **arg);
int cgiDS18b20(HttpdConnData *connData);
int cgiState(HttpdConnData *connData);
int cgiUI(HttpdConnData *connData);
void tplUI(HttpdConnData *connData, char *token, void **arg);
void tplMQTT(HttpdConnData *connData, char *token, void **arg);
int cgiMQTT(HttpdConnData *connData);
void tplHTTPD(HttpdConnData *connData, char *token, void **arg);
int cgiHTTPD(HttpdConnData *connData);
void tplBroadcastD(HttpdConnData *connData, char *token, void **arg);
int cgiBroadcastD(HttpdConnData *connData);
void tplConfig(HttpdConnData *connData, char *token, void **arg);
int cgiConfig(HttpdConnData *connData);
int tplIndex(HttpdConnData *connData, char *token, void **arg);
void tplNTP(HttpdConnData *connData, char *token, void **arg);
int cgiNTP(HttpdConnData *connData);
int cgiReset(HttpdConnData *connData);
void tplRLYSettings(HttpdConnData *connData, char *token, void **arg);
int cgiRLYSettings(HttpdConnData *connData);
void tplSensorSettings(HttpdConnData *connData, char *token, void **arg);
int cgiSensorSettings(HttpdConnData *connData);
int cgiMax31855(HttpdConnData *connData);
int tplMax31855(HttpdConnData *connData, char *token, void **arg);
int cgisi7020(HttpdConnData *connData);
int tplsi7020(HttpdConnData *connData, char *token, void **arg);
int cgimlx91205(HttpdConnData *connData);
int tplmlx91205(HttpdConnData *connData, char *token, void **arg);
int cgiws2812b(HttpdConnData *connData);
void tplws2812b(HttpdConnData *connData, char *token, void **arg);

#endif
