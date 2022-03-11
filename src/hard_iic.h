
#ifndef __HARD_IIC_H__
#define __HARD_IIC_H__

#include <stm32f10x.h>

#define NO_ACK 0 /*/表示发送不应答信号*/
#define ACK 1 /*表示发送应答信号 */



void iic_init(I2C_TypeDef* I2Cx);

int8_t iic_start(I2C_TypeDef* I2Cx);

void iic_stop(I2C_TypeDef* I2Cx);

int8_t iic_put_byte_data(I2C_TypeDef* I2Cx,uint8_t dat);

uint8_t iic_get_ack(I2C_TypeDef* I2Cx);


uint16_t iic_get_byte_data(I2C_TypeDef* I2Cx);

void iic_send_ack(I2C_TypeDef* I2Cx,uint8_t ack);

int8_t iic_put_devaddr(I2C_TypeDef* I2Cx,uint8_t addr);


#endif

