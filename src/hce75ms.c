
/*
	2021-11-30 hce75ms用于测量主板和cpu温度
	
	iic1通信方式
	
	cpu温度的地址 0x91/0x90  对应读写
	主板温度的地址 0x93/0x92  对应读写
	
*/
#include "includes.h"



#define HCE75_CPU_TEMP_ADDR 0x90
#define HCE75_BOARD_TEMP_ADDR 0x92

/*
	传感器iic控制器初始化，并配置为13位模式！！！
	成功返回0，否则返回-1
*/
int hce75ms_init(void)
{
	int ret = 0;
//	uint8_t dat[2] = {0x00,0x30}; //配置为13位模式
	//控制器初始化
	IICapp_init(I2C1);   //
		
	//配置hce模式13位模式
	//ret = IICapp_write_bytes(I2C1,HCE75_CPU_TEMP_ADDR,1,dat,2);
	//ret += IICapp_write_bytes(I2C1,HCE75_BOARD_TEMP_ADDR,1,dat,2);   //主板也配置，这样计算方便点	
	return ret;
}

//读cpu的温度(整型温度返回后应该*0.0625才是实际温度)
//通过参数返回温度值
//成功返回0，否则返回-1
int read_cpu_temp(short* temp)
{
	uint8_t dat[2];
	if(0 == IICapp_read_bytes(I2C1,HCE75_CPU_TEMP_ADDR,0,dat,2))   //手册上没说是可以进行随机数据读取的，不知是否可行？？？
	{
		*temp= dat[0]<<8 | dat[1];   //
		*temp >>= 4;   //低4位丢弃！！！ 右移的好处是能够保留符号位！！
		return 0;
	}
	return -1;
}

//读board的温度(整型温度返回后应该*0.0625才是实际温度)
//通过参数返回温度值
//成功返回0，否则返回-1
int read_board_temp(short* temp)
{
	uint8_t dat[2];
	
	if(0 == IICapp_read_bytes(I2C1,HCE75_BOARD_TEMP_ADDR,0,dat,2))
	{
		*temp= dat[0]<<8 | dat[1];   //
		*temp >>= 4;   //低4位丢弃！！！ 右移的好处是能够保留符号位！！
		return 0;
	}
	return -1;
}
