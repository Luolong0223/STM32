#ifndef _AHT20_H_
#define _AHT20_H_

#include "main.h"
#include "spi.h"

extern I2C_HandleTypeDef hi2c1;

#define AHT20_IIC       hi2c1
#define AHT20_IIC_ADDR  0x70
#define AHT20_IIC_BUS_WAIT  0XFFFFFF

//extern void AHT20_Read_CTdata(uint32_t *ct); //没有CRC校验，直接读取AHT20的温度和湿度数据
//extern void AHT20_Read_CTdata_crc(uint32_t *ct); //CRC校验后，读取AHT20的温度和湿度数据
//extern void JH_Reset_REG(uint8_t addr);///重置寄存器

extern uint8_t AHT20_Read_Status(void);//读取AHT20的状态寄存器
extern void AHT20_Start_Init(void);///上电初始化进入正常测量状态
extern void AHT20_Init(void);   //初始化AHT20
extern  void Aht20_Task_Start(void);

extern  void Aht20_Task_Read(void);
//extern void Aht20_Task_Read( uint16_t H,uint16_t T);


extern void Aht20_Reset( void ); //软复位

extern float  Temperature;
extern float  Humidity;  //真实数据，如果无线传递建议放大10倍后转成整形

#endif



