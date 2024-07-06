
#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "main.h"

typedef struct 
{
 	uint8_t  recount;      //modbus端口接收到的数据个数
 	uint8_t  reflag;       //modbus一帧数据接受完成标志位
 	uint8_t 	CRC_OK;    //接收数据后处理完毕
	
	uint8_t		TX_ID;						//解析ID
	uint8_t		TX_Feature;				//解析功能码
	uint8_t		TX_Address[2];		//解析地址
	uint8_t		TX_DatLen[2];				//数据长度
	uint8_t		TX_DataCont;			//数据字节数量
	uint8_t 	TX_Data[250];	//解析数据类容
	uint8_t		TX_Verify[2];			//解析校验码
	uint8_t  	TX_Buf[250]; 			//modbus接发送缓冲区
	
	uint8_t		RX_ID;						//解析ID
	uint8_t		RX_Feature;				//解析功能码
	uint8_t		RX_Address[2];		//解析地址
	uint8_t		RX_DatLen;				//数据长度
	uint8_t 	RX_Data[250];	//解析数据类容
	uint8_t		RX_Verify[2];			//解析校验码
	uint8_t  	RX_Buf[250]; 			//modbus接发送缓冲区
	
}MODBUS;
 

extern MODBUS modbus;

uint16_t ModbusCRC(uint8_t *buf, int len);   //计算CRC
int VerifyModbusCRC(uint8_t *data, int len); //校验CRC
void HMI_Send(uint8_t id);  //HMI通讯发送帧0x10
void HMI_Read(uint8_t id);
void Modbus_Analysis(uint8_t*	Data,uint8_t	Len);  //modbus接收解析
void Modbus_03Read(uint8_t ID,uint16_t START_ADDRESS,uint16_t DataLen); //03查询码
void Modbus_Init(void);

#endif
