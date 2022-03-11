#ifndef __DS18B20_H
#define __DS18B20_H 
  
#include "stm32f10x.h"


//DS18B20ָ��
typedef enum   
{  
    SEARCH_ROM          =   0xf0,   //����ROMָ��  
    READ_ROM            =   0x33,   //��ȡROMָ��
    MATH_ROM            =   0x55,   //ƥ��ROMָ��
    SKIP_ROM            =   0xcc,   //����ROMָ��
    ALARM_SEARCH        =   0xec,   //��������ָ�� 
    CONVERT_T           =   0x44,   //�¶�ת��ָ��
    WRITE_SCRATCHPAD    =   0x4e,   //д�ݴ���ָ��
    READ_SCRATCHPAD     =   0xbe,   //��ȡת����ָ��
    COPY_SCRATCHPAD     =   0x48,   //�����ݴ���ָ��  
    RECALL_E2           =   0xb8,   //�ٻ�EEPROMָ��
    READ_POWER_SUPPLY   =   0xb4,   //��ȡ��Դģʽָ��  
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
