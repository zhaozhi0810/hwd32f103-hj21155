

#ifndef __18B20_H__
#define __18B20_H__

#include <stm32f10x.h>

//extern unsigned char DS18B20_SensorNum;


int32_t ns18b20_init(void);
int ns18b20_read(u8 i,short *temperature);
int NS18B20StartConvert(void);   //¿ªÊ¼×ª»»

#endif

