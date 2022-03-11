#ifndef __DS18B20_H
#define __DS18B20_H 
  
#include "stm32f10x.h"


//DS18B20指令
typedef enum   
{  
    SEARCH_ROM          =   0xf0,   //搜索ROM指令  
    READ_ROM            =   0x33,   //读取ROM指令
    MATH_ROM            =   0x55,   //匹配ROM指令
    SKIP_ROM            =   0xcc,   //跳过ROM指令
    ALARM_SEARCH        =   0xec,   //报警搜索指令 
    CONVERT_T           =   0x44,   //温度转换指令
    WRITE_SCRATCHPAD    =   0x4e,   //写暂存器指令
    READ_SCRATCHPAD     =   0xbe,   //读取转存器指令
    COPY_SCRATCHPAD     =   0x48,   //拷贝暂存器指令  
    RECALL_E2           =   0xb8,   //召回EEPROM指令
    READ_POWER_SUPPLY   =   0xb4,   //读取电源模式指令  
} DS18B20_CMD;  




u8 DS18B20_Init(void);
u8 DS18B20_Read_Byte(void);
u8 DS18B20_Read_Bit(void);
u8 DS18B20_Answer_Check(void);
void  DS18B20_GPIO_Config(void);
void  DS18B20_Mode_IPU(void);
void  DS18B20_Mode_Out(void);
void  DS18B20_Rst(void);
void  DS18B20_Search_Rom(void);
void  DS18B20_Write_Byte(u8 dat);
short DS18B20_Get_Temp(u8 i);
int ns18b20_read(u8 i,short *temperature);
int NS18B20StartConvert(void);

#endif
