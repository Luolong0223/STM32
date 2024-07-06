
#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "main.h"

typedef struct 
{
 	uint8_t  recount;      //modbus�˿ڽ��յ������ݸ���
 	uint8_t  reflag;       //modbusһ֡���ݽ�����ɱ�־λ
 	uint8_t 	CRC_OK;    //�������ݺ������
	
	uint8_t		TX_ID;						//����ID
	uint8_t		TX_Feature;				//����������
	uint8_t		TX_Address[2];		//������ַ
	uint8_t		TX_DatLen[2];				//���ݳ���
	uint8_t		TX_DataCont;			//�����ֽ�����
	uint8_t 	TX_Data[250];	//������������
	uint8_t		TX_Verify[2];			//����У����
	uint8_t  	TX_Buf[250]; 			//modbus�ӷ��ͻ�����
	
	uint8_t		RX_ID;						//����ID
	uint8_t		RX_Feature;				//����������
	uint8_t		RX_Address[2];		//������ַ
	uint8_t		RX_DatLen;				//���ݳ���
	uint8_t 	RX_Data[250];	//������������
	uint8_t		RX_Verify[2];			//����У����
	uint8_t  	RX_Buf[250]; 			//modbus�ӷ��ͻ�����
	
}MODBUS;
 

extern MODBUS modbus;

uint16_t ModbusCRC(uint8_t *buf, int len);   //����CRC
int VerifyModbusCRC(uint8_t *data, int len); //У��CRC
void HMI_Send(uint8_t id);  //HMIͨѶ����֡0x10
void HMI_Read(uint8_t id);
void Modbus_Analysis(uint8_t*	Data,uint8_t	Len);  //modbus���ս���
void Modbus_03Read(uint8_t ID,uint16_t START_ADDRESS,uint16_t DataLen); //03��ѯ��
void Modbus_Init(void);

#endif
