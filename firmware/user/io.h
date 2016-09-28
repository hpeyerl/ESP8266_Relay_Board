void ICACHE_FLASH_ATTR ioGPIO(int ena,int gpio);
void ioInit(void);
extern char currGPIO12State;
extern char currGPIO13State;
extern char currGPIO15State;
extern char relay1GPIO, relay2GPIO, relay3GPIO;
int getRelaystate(int);
