
#ifndef __HCE75MS_H__
#define __HCE75MS_H__

int hce75ms_init(void);


//读cpu的温度(整型温度返回后应该*0.0625才是实际温度)
//通过参数返回温度值
//成功返回0，否则返回-1
int read_cpu_temp(short* temp);


//读board的温度(整型温度返回后应该*0.0625才是实际温度)
//通过参数返回温度值
//成功返回0，否则返回-1
int read_board_temp(short* temp);


#endif



