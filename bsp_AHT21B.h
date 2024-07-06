#ifndef _AHT20_H_
#define _AHT20_H_

#include "main.h"
#include "spi.h"

extern I2C_HandleTypeDef hi2c1;

#define AHT20_IIC       hi2c1
#define AHT20_IIC_ADDR  0x70
#define AHT20_IIC_BUS_WAIT  0XFFFFFF

//extern void AHT20_Read_CTdata(uint32_t *ct); //û��CRCУ�飬ֱ�Ӷ�ȡAHT20���¶Ⱥ�ʪ������
//extern void AHT20_Read_CTdata_crc(uint32_t *ct); //CRCУ��󣬶�ȡAHT20���¶Ⱥ�ʪ������
//extern void JH_Reset_REG(uint8_t addr);///���üĴ���

extern uint8_t AHT20_Read_Status(void);//��ȡAHT20��״̬�Ĵ���
extern void AHT20_Start_Init(void);///�ϵ��ʼ��������������״̬
extern void AHT20_Init(void);   //��ʼ��AHT20
extern  void Aht20_Task_Start(void);

extern  void Aht20_Task_Read(void);
//extern void Aht20_Task_Read( uint16_t H,uint16_t T);


extern void Aht20_Reset( void ); //��λ

extern float  Temperature;
extern float  Humidity;  //��ʵ���ݣ�������ߴ��ݽ���Ŵ�10����ת������

#endif



