
/*
	2021-09-24
	第九个移植的文件。
	
	这一层封装在IIC的底层结构上。不涉及iic时序。
	
*/

#include "includes.h"
#include "sm2990.h"

#define SM2990_DEV_ADDR_BASE 0x98    //高5位有效




#define SM2990_STATUS_REG                  0x00    //!< Indicates BUSY state and conversion status
#define SM2990_CONTROL_REG                 0x01    //!< Controls Mode, Single/Repeat, Celsius/Kelvin
#define SM2990_TRIGGER_REG                 0x02    //!< Triggers a conversion
#define SM2990_TINT_MSB_REG                0x04    //!< Internal Temperature MSB
#define SM2990_TINT_LSB_REG                0x05    //!< Internal Temperature LSB
#define SM2990_V1_MSB_REG                  0x06    //!< V1, V1-V2, or T_R1 T MSB
#define SM2990_V1_LSB_REG                  0x07    //!< V1, V1-V2, or T_R1 T LSB
#define SM2990_V2_MSB_REG                  0x08    //!< V2, V1-V2, or T_R2 Voltage MSB
#define SM2990_V2_LSB_REG                  0x09    //!< V2, V1-V2, or T_R2 Voltage LSB
#define SM2990_V3_MSB_REG                  0x0A    //!< V3, V3-V4, or T_R2 T MSB
#define SM2990_V3_LSB_REG                  0x0B    //!< V3, V3-V4, or T_R2 T LSB
#define SM2990_V4_MSB_REG                  0x0C    //!< V4, V3-V4, or T_R2 Voltage MSB
#define SM2990_V4_LSB_REG                  0x0D    //!< V4, V3-V4, or T_R2 Voltage LSB
#define SM2990_VCC_MSB_REG                 0x0E    //!< Vcc MSB
#define SM2990_VCC_LSB_REG                 0x0F    //!< Vcc LSB

/*! @} */
/*! @name SM2990_CONTROL_REG SETTINGS
    Bitwise OR settings, and write to SM2990_CONTROL_REG to configure settings.
    Bitwise AND with value read from SM2990_CONTROL_REG to determine present setting.
@{ */

#define SM2990_KELVIN_ENABLE                 0x80 //!< Enable for Kelvin. 
#define SM2990_CELSIUS_ENABLE                0x00 //!< Enable for Celsius.
#define SM2990_TEMP_FORMAT_MASK              0x80 //!< Use mask when changing temp formats

#define SM2990_SINGLE_ENABLE                 0x40 //!< Enable for Single Acquisition
#define SM2990_REPEATED_ENABLE               0x00 //!< Enable for Repeated Acquisition Mode
#define SM2990_ACQUISITION_MASK              0x40 //!< Use mask when changing acquisition settings


/*! @} */
/*! @name SM2990_CONTROL_REG ENABLE
    Bitwise AND 0xE7 then Bitwise OR settings, and write to SM2990_CONTROL_REG to configure enable settings.
    Bitwise AND with value read from SM2990_CONTROL_REG to determine present enable setting.
@{ */

#define SM2990_ENABLE_INT_TEMPERATURE        0x00 //!< Read only Internal Temperature
#define SM2990_ENABLE_V1                     0x08 //!< Tr1, V1 or V1-V2 per Mode are enabled
#define SM2990_ENABLE_V2                     0x10 //!< Tr2, V3 or V3-V4 per Mode are enabled
#define SM2990_ENABLE_ALL                    0x18 //!< All measurements per Mode are enabled 
#define SM2990_TEMP_MEAS_MODE_MASK           0x18 //!< Use mask when changing temp meas modes

/*! @} */
/*! @name SM2990_CONTROL_REG MODE
    Bitwise AND 0xF8 then Bitwise OR settings, and write to SM2990_CONTROL_REG to configure enable settings.
    Bitwise AND with value read from SM2990_CONTROL_REG to determine present enable setting.
@{ */

#define SM2990_V1_V2_TR2                   0x00 //!< Read V1, V2 and TR2
#define SM2990_V1V2_TR2                    0x01 //!< Read V1-V2 and TR2
#define SM2990_V1V2_V3_V4                  0x02 //!< Read V1-V2, V3 and V4
#define SM2990_TR1_V3_V4                   0x03 //!< Read TR1, V3 and V4
#define SM2990_TR1_V3V4                    0x04 //!< Read TR1 and V3-V4
#define SM2990_TR1_TR2                     0x05 //!< Read TR1 and TR2
#define SM2990_V1V2_V3V4                   0x06 //!< Read V1-V2 and V3-V4 
#define SM2990_V1_V2_V3_V4                 0x07 //!< Read V1, V2, V3 and V4
#define SM2990_VOLTAGE_MODE_MASK           0x07 //!< Use mode mask when changing modes

const I2C_TypeDef* SM2990_IIC_CONTROLER = I2C1;
#define SM2990_DATA_LENGTH 2




// Reads an 8-bit register from the SM2990 using the standard repeated start format.
static int8_t SM2990_register_read(uint8_t i2c_address, uint8_t register_address, uint8_t *register_data)
{
	int8_t ack = 0;
	ack = IICapp_read_bytes((I2C_TypeDef*)SM2990_IIC_CONTROLER,
									SM2990_DEV_ADDR_BASE | i2c_address,
										register_address,register_data,
											1);
//	ack = i2c_read_byte_data(SM2990_DEV_ADDR_BASE | i2c_address, register_address, register_data);
	return(ack);
}

// Write one byte to an SM2990 register.
// Writes to an 8-bit register inside the SM2990 using the standard I2C repeated start format.
static int8_t SM2990_register_write(uint8_t i2c_address, uint8_t register_address, uint8_t register_data)
{
	int8_t ack = 0;
	ack = IICapp_write_bytes((I2C_TypeDef*)SM2990_IIC_CONTROLER,
									SM2990_DEV_ADDR_BASE | i2c_address,
										register_address,&register_data,
											1);
	//  ack = i2c_write_byte_data(SM2990_DEV_ADDR_BASE | i2c_address, register_address, register_data);
	return(ack);
}




void sm2990_init(void)
{	
	//1. 初始化控制器
	IICapp_init(I2C1);  //控制器初始化		

	//2. 给每一个2990，配置相应的工作模式。
	SM2990_register_write(SM2990_DEV_ADDR_VOL, SM2990_TRIGGER_REG,SM2990_ENABLE_ALL | SM2990_V1_V2_V3_V4);   //默认连续转换模式
	SM2990_register_write(SM2990_DEV_ADDR_CPU_TEMP, SM2990_TRIGGER_REG,  SM2990_ENABLE_ALL | SM2990_TR1_TR2);
	SM2990_register_write(SM2990_DEV_ADDR_LCD_TEMP, SM2990_TRIGGER_REG,  SM2990_ENABLE_ALL | SM2990_TR1_TR2);
	
	
}



// Reads a 14-bit adc_code from SM2990.
int8_t SM2990_adc_read_v1v4(uint8_t i2c_address, uint8_t *dat, uint8_t len)
{
	int8_t ack = 0;
	
	/* 0 表示返回成功， 非0 表示返回失败*/
	ack = IICapp_read_bytes((I2C_TypeDef*)SM2990_IIC_CONTROLER,
									SM2990_DEV_ADDR_BASE | i2c_address,
										SM2990_V1_MSB_REG,dat,
											SM2990_DATA_LENGTH*len);
	//ack = i2c_read_word_data(i2c_address, msb_register_address, &dat);

	return(ack);
}





// 手动触发一次转换，在单次模式下使用！！！！
//int8_t SM2990_triggle_a_conversion(void)
//{
//	SM2990_register_write(SM2990_DEV_ADDR_VOL, SM2990_CONTROL_REG,0x11);   //单次转换模式
//	SM2990_register_write(SM2990_DEV_ADDR_CPU_TEMP, SM2990_CONTROL_REG, 0x11);  //写入任意值都会触发转换
//	SM2990_register_write(SM2990_DEV_ADDR_LCD_TEMP, SM2990_CONTROL_REG, 0x11);
//}


// Reads a 14-bit adc_code from SM2990.
//int8_t SM2990_adc_read(uint8_t i2c_address, uint8_t msb_register_address, int16_t *adc_code, int8_t *data_valid)
//{
//	int8_t ack = 0;
//	uint8_t dat[2];
//	uint16_t code;
//	
//	ack = IICapp_read_bytes((I2C_TypeDef*)SM2990_IIC_CONTROLER,
//									SM2990_DEV_ADDR_BASE | i2c_address,
//										msb_register_address,dat,
//											SM2990_DATA_LENGTH);
//	//ack = i2c_read_word_data(i2c_address, msb_register_address, &dat);
//	code = dat[0]<<8 | dat[1];

//	*data_valid = (code >> 15) & 0x01;   // Place Data Valid Bit in *data_valid
//	*adc_code = code & 0x7FFF;  // Removes data valid bit to return proper adc_code value

//	return(ack);
//}





// Reads a 14-bit adc_code from the SM2990 but enforces a maximum timeout.
// Similar to SM2990_adc_read except it repeats until the data_valid bit is set, it fails to receive an I2C acknowledge, or the timeout (in milliseconds)
// expires. It keeps trying to read from the SM2990 every millisecond until the data_valid bit is set (indicating new data since the previous
// time this register was read) or until it fails to receive an I2C acknowledge (indicating an error on the I2C bus).
//int8_t SM2990_adc_read_timeout(uint8_t i2c_address, uint8_t msb_register_address, int16_t *adc_code, int8_t *data_valid, uint16_t timeout, uint8_t status_bit)
//{
//	int8_t ack = 0;
//	uint8_t reg_data;
//	uint16_t timer_count;  // Timer count for data_valid
//	*data_valid = 0; 

//	for (timer_count = 0; timer_count < timeout; timer_count++)
//	{
//		//! 1)Read status register until correct data valid bit is set
//		ack |=  SM2990_register_read(i2c_address, SM2990_STATUS_REG, &reg_data); 
//		if ((ack) || (((reg_data>>status_bit)&0x1)==1)){
//			break;
//		}
//		delay_1ms(1);
//	}
//	ack |= SM2990_adc_read(i2c_address, msb_register_address, &(*adc_code), &(*data_valid));   //! 2) It's either valid or it's timed out, we read anyways
//	if(*data_valid  !=1){
//		return (1); 
//	}
//	return(ack);
//}

// Reads new data (even after a mode change) by flushing old data and waiting for the data_valid bit to be set.
// This function simplifies adc reads when modes are changing.  For example, if V1-V2 changes from temperature mode
// to differential voltage mode, the data in the register may still correspond to the temperature reading immediately
// after the mode change.  Flushing one reading and waiting for a new reading guarantees fresh data is received.
// If the timeout is reached without valid data (*data_valid=1) the function exits.
//int8_t SM2990_adc_read_new_data(uint8_t i2c_address, uint8_t msb_register_address, int16_t *adc_code, int8_t *data_valid, uint16_t timeout)
//{
//  int8_t ack = 0; 
//  uint8_t status_bit; 
//  status_bit  = msb_register_address/2-1;
//  ack |= SM2990_adc_read_timeout(i2c_address, msb_register_address, adc_code, data_valid, timeout, status_bit); //! 1)  Throw away old data
// 
//  ack |= SM2990_adc_read_timeout(i2c_address, msb_register_address, adc_code, data_valid, timeout,status_bit); //! 2) Read new data
//  
//  return(ack);
//}



//// Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
//// bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
//int8_t SM2990_register_set_clear_bits(uint8_t i2c_address, uint8_t register_address, uint8_t bits_to_set, uint8_t bits_to_clear)
//{
//	uint8_t register_data;
//	int8_t ack = 0;

//	ack |= SM2990_register_read(i2c_address, register_address, &register_data);  //! 1) Read register
//	register_data = register_data & (~bits_to_clear); //! 2) Clear bits that were set to be cleared
//	register_data = register_data | bits_to_set;
//	ack |= SM2990_register_write(i2c_address, register_address, register_data);  //! 3) Write to register with the cleared bits
//	return(ack);
//}









// Calculates the SM2990 single-ended input voltages
float SM2990_code_to_single_ended_voltage(int16_t adc_code, float SM2990_single_ended_lsb)
{
	float voltage;
	int16_t sign = 1;
	if (adc_code >> 14)
	{
		adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1) Converts two's complement to binary
		sign = -1;
	}
	adc_code = (adc_code & 0x3FFF);
	voltage = ((float) adc_code) * SM2990_single_ended_lsb * sign;   //! 2) Convert code to voltage from lsb
	return (voltage);
}


// Calculates the SM2990 temperature
float SM2990_temperature(int16_t adc_code, float SM2990_temperature_lsb, int8_t unit)
{
	float temperature;
	adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
	
	if(!unit){                                                     //! 2)Checks to see if it's Kelvin
		if(adc_code >>12)
		{
			adc_code = (adc_code | 0xE000);                         //! Sign extend if it's not Kelvin (Celsius)
		}
	}
	temperature = ((float) adc_code) * SM2990_temperature_lsb;   //! 3) Converts code to temperature from temperature lsb

	return (temperature);
}


// Calculates the SM2990 temperature,只有整数部分，小数部分不考虑，摄氏温度计算
int16_t SM2990_temperature_int(int16_t adc_code)
{
	int16_t temperature;
	adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
	

	if(adc_code >>12)  //符号位为1，表示负数
	{
		adc_code = (adc_code | 0xE000);                         //! Sign extend if it's not Kelvin (Celsius)
	}
	
	temperature = adc_code>>4;   //去掉低4位小数
	if((adc_code & 0xf) > 7)
		temperature ++;  		//四舍五入			 //! 3) Converts code to temperature from temperature lsb

	
	return (temperature);
}



//以下代码上位机可以使用，单片机只负责读数据，就不用转换出实际的数值了。
//建议上位机参考
#if 0
// Calculates the SM2990 Vcc voltage
float SM2990_code_to_vcc_voltage(int16_t adc_code, float SM2990_single_ended_lsb)
{
	float voltage;
	int16_t sign = 1;
	
	if (adc_code >> 14)
	{
		adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1) Converts two's complement to binary
		sign = -1;
	}

	voltage = (((float) adc_code) * SM2990_single_ended_lsb * sign) + 2.5; //! 2) Convert code to Vcc Voltage from single-ended lsb
	return (voltage);
}

// Calculates the SM2990 differential input voltage.
float SM2990_code_to_differential_voltage(int16_t adc_code, float SM2990_differential_lsb)
{
	float voltage;
	int16_t sign = 1;
	
	if (adc_code >> 14)
	{
		adc_code = (adc_code ^ 0x7FFF) + 1;                 //! 1)Converts two's complement to binary
		sign = -1;
	}
	voltage = ((float) adc_code) * SM2990_differential_lsb * sign;   //! 2) Convert code to voltage form differential lsb
	return (voltage);
}



//  Calculates the SM2990 diode voltage
float SM2990_code_to_diode_voltage(int16_t adc_code, float SM2990_diode_voltage_lsb)
{
  float voltage;
  adc_code = (adc_code & 0x1FFF);                               //! 1) Removes first 3 bits
  voltage = ((float) adc_code) * SM2990_diode_voltage_lsb;     //! 2) Convert code to voltage from diode voltage lsb
  return (voltage);
}
#endif
