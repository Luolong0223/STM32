#include "bsp_modbus.h"
#include "stdio.h"
#include "usart.h"
//#include "tim.h"
#include <stdint.h>

MODBUS modbus;
#define MessageLen 72   //д��HMI����������
uint16_t HMI_Message_Send[MessageLen];
uint8_t  HMI_Message_Send_8b[MessageLen*2];
uint16_t HMI_Message_Read[10];
uint8_t  HMI_Message_Read_8b[20];

// CRC���㺯��
uint16_t ModbusCRC(uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF; // CRC�ĳ�ʼֵ
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos]; // �������ֽ���CRC�Ĵ������
        for (int i = 8; i != 0; i--) { // ����ÿ��λ
            if ((crc & 0x0001) != 0) { // ���CRC�Ĵ��������λΪ1
                crc >>= 1; // ����һλ
                crc ^= 0xA001; // �����ʽA001��1010 0000 0000 0001�����
            } else {
                crc >>= 1; // ������λΪ0��ֻ������һλ
            }
        }
    }
    // ���ؼ���õ���CRCֵ
    return crc;
}
// ��֤CRC�ĺ���
int VerifyModbusCRC(uint8_t *data, int len) 
{
    if (len < 2) {
        // ���ݳ������ٰ��������ֽڣ�CRC��
        return 0;
    }

		// ����������ȡCRCֵ
    uint16_t receivedCRC = (uint16_t)(data[len - 1] << 8 | data[len - 2]);
		
    // ����CRC
    uint16_t calculatedCRC = ModbusCRC(data, len - 2);

    // �ȽϽ��յ���CRC�ͼ���õ���CRC

		 return (receivedCRC == calculatedCRC);
}

void	HMI_Send(uint8_t id)  //HMIͨѶ����֡0x10
{
	uint16_t crc;
	uint8_t T_DA[200];
	uint8_t i;
	for (int i = 0; i < sizeof(HMI_Message_Send); i++) 
	{
     HMI_Message_Send_8b[2 * i] = (unsigned char)(HMI_Message_Send[i] >> 8);     // ��ȡ��8λ
     HMI_Message_Send_8b[2 * i + 1] = (unsigned char)(HMI_Message_Send[i] & 0xFF); // ��ȡ��8λ
  }
	modbus.TX_ID	 = id;
	modbus.TX_Feature = 0x10;
	modbus.TX_Address[0] = 0x00;							//��ʼ��ַ
	modbus.TX_Address[1] = 0x00;
	modbus.TX_DatLen[0] = (sizeof(HMI_Message_Send_8b)/2)/256;		//���ݳ���
	modbus.TX_DatLen[1] = (sizeof(HMI_Message_Send_8b)/2)%256;
	modbus.TX_DataCont = sizeof(HMI_Message_Send_8b)%256;				//�����ֽڸ���
	for(uint8_t i=0;i<sizeof(HMI_Message_Send_8b);i++)
	{
		modbus.TX_Data[i] = HMI_Message_Send_8b[i];
	}
	T_DA[0] = modbus.TX_ID;
	T_DA[1] = modbus.TX_Feature;
	T_DA[2] = modbus.TX_Address[0];
	T_DA[3] = modbus.TX_Address[1];
	T_DA[4] = modbus.TX_DatLen[0];
	T_DA[5] = modbus.TX_DatLen[1];
	T_DA[6] = modbus.TX_DataCont;
	for(i=0;i<sizeof(HMI_Message_Send_8b);i++)
	{
		T_DA[i+7] = HMI_Message_Send_8b[i];
	}
	crc = ModbusCRC(T_DA,sizeof(HMI_Message_Send_8b)+7);
	modbus.TX_Verify[0] = (uint8_t)crc & 0xff;
	modbus.TX_Verify[1] = (uint8_t)(crc >> 8) & 0xff;
	for(i=0;i<sizeof(HMI_Message_Send_8b)+7;i++)
	{
		modbus.TX_Buf[i] = T_DA[i];
	}
	modbus.TX_Buf[sizeof(HMI_Message_Send_8b)+7] = modbus.TX_Verify[0];
	modbus.TX_Buf[sizeof(HMI_Message_Send_8b)+7+1] = modbus.TX_Verify[1];
	modbus.reflag = 1;		//HMI�������ݵȴ���Ӧ����Ӧ��� = 0
	HAL_UART_Transmit_DMA(&huart2,modbus.TX_Buf,sizeof(HMI_Message_Send_8b)+9);
	//HAL_UART_Transmit(&huart1,modbus.TX_Buf,sizeof(HMI_Message_Send_8b)+9,0xffff);	//UART1����MODBUS����  ����
}


void	HMI_Read(uint8_t id)      //��ȡHMI�������
{
	uint16_t crc;
	uint8_t T_DA[6];
	uint8_t i;

	modbus.TX_ID	 = id;
	modbus.TX_Feature = 0x04;
	modbus.TX_Address[0] = 0x00;							//��ʼ��ַ
	modbus.TX_Address[1] = 0x00;
	modbus.TX_DatLen[0] = (sizeof(HMI_Message_Read_8b)/2)/256;		//���ݳ���
	modbus.TX_DatLen[1] = (sizeof(HMI_Message_Read_8b)/2)%256;

	T_DA[0] = modbus.TX_ID;
	T_DA[1] = modbus.TX_Feature;
	T_DA[2] = modbus.TX_Address[0];
	T_DA[3] = modbus.TX_Address[1];
	T_DA[4] = modbus.TX_DatLen[0];
	T_DA[5] = modbus.TX_DatLen[1];

	crc = ModbusCRC(T_DA,sizeof(T_DA));
	
	modbus.TX_Verify[0] = (uint8_t)crc & 0xff;
	modbus.TX_Verify[1] = (uint8_t)(crc >> 8) & 0xff;
	
	for(i=0;i<sizeof(T_DA);i++)
	{
		modbus.TX_Buf[i] = T_DA[i];
	}
	modbus.TX_Buf[sizeof(T_DA)] = modbus.TX_Verify[0];
	modbus.TX_Buf[sizeof(T_DA)+1] = modbus.TX_Verify[1];
	modbus.reflag = 2;
	HAL_UART_Transmit_DMA(&huart2,modbus.TX_Buf,sizeof(T_DA)+2);
}


void Modbus_03Read(uint8_t ID,uint16_t START_ADDRESS,uint16_t DataLen) //03��ѯ��
{
	uint16_t crc;
	uint8_t T_DA[6];
	uint8_t i;
	
	modbus.TX_ID 				 = ID;
	modbus.TX_Feature 	 = 0x03;
	modbus.TX_Address[0] = (unsigned char) (START_ADDRESS >> 8); 		// ��ȡ��8λ
	modbus.TX_Address[1] = (unsigned char) (START_ADDRESS & 0xFF);   // ��ȡ��8λ
	modbus.TX_DatLen[0]  = (unsigned char) (DataLen >> 8); 		// ��ȡ��8λ
	modbus.TX_DatLen[1]  = (unsigned char) (DataLen & 0xFF);   // ��ȡ��8λ
	
	T_DA[0] = modbus.TX_ID;
	T_DA[1] = modbus.TX_Feature;
	T_DA[2] = modbus.TX_Address[0];
	T_DA[3] = modbus.TX_Address[1];
	T_DA[4] = modbus.TX_DatLen[0];
	T_DA[5] = modbus.TX_DatLen[1];
	crc = ModbusCRC(T_DA,sizeof(T_DA));
	modbus.TX_Verify[0] = (uint8_t)crc & 0xff;
	modbus.TX_Verify[1] = (uint8_t)(crc >> 8) & 0xff;
	for(i=0;i<sizeof(T_DA);i++)
	{
		modbus.TX_Buf[i] = T_DA[i];
	}
	modbus.TX_Buf[sizeof(T_DA)] = modbus.TX_Verify[0];
	modbus.TX_Buf[sizeof(T_DA)+1] = modbus.TX_Verify[1];
	if(modbus.TX_ID == 0x65)
	{
		modbus.reflag = 4;	
	}
	else
		modbus.reflag = 3;	
		
	HAL_UART_Transmit_DMA(&huart2,modbus.TX_Buf,sizeof(T_DA)+2);
	//HAL_UART_Transmit(&huart1,modbus.TX_Buf,sizeof(T_DA)+2,0xffff);
}


void Modbus_Manage(void)	//MODBUS�������ݴ���
{
	switch (modbus.reflag)
  {
  	case 1:
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//HMI����CRC��֤�ɹ�
							if(modbus.RX_Feature == 0x10) //�жϷ��ع�����
							{
								printf("HMI����������ɣ�");
								modbus.reflag = 0;		//HMI������Ӧ���
							}
							else
							{
								//HAL_UART_Transmit(&huart1,(uint8_t*)"HMI�������ݴ���",17,0xffff);
								printf("HMI����������ɣ�");
							}
						}
						break;
  	case 2:
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//HMI����CRC��֤�ɹ�
							if(modbus.RX_Feature == 0x04) //�жϷ��ع�����
							{
								
								for (int i = 0; i < sizeof(HMI_Message_Read); i++) 
								{
									//������8λ����ת����16λ		
									HMI_Message_Read[i] = (uint8_t)modbus.RX_Data[2*i] << 8 | (uint8_t)modbus.RX_Data[2*i+1];  			
								}
								
								printf("HMI���ݲ�ѯ��ɣ�");
								modbus.reflag = 0;		//HMI������Ӧ���
							}
							else
							{
								printf("HMI���ݲ�ѯʧ�ܣ�");
							}
						}
						break;
			
		case 3: 
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//����CRC��֤�ɹ�
							if(modbus.RX_Feature == 0x03) //�жϷ��ع�����
							{
								//��8λ����תΪ16λ����
								HMI_Message_Send[modbus.RX_ID-1] = (uint16_t)(modbus.RX_Data[0]<<8 | modbus.RX_Data[1]);  
								printf("03���ݲ�ѯ��ɣ�ID:%d",modbus.RX_ID);
								modbus.reflag = 0;		//������Ӧ���
							}
							else
							{
								printf("03���ݲ�ѯʧ�ܣ�ID:%d",modbus.TX_ID);
							}
						}
						break;
		case 4: 
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//����CRC��֤�ɹ�
							if(modbus.RX_Feature == 0x03) //�жϷ��ع�����
							{
								for(int i=0;i<(modbus.RX_DatLen/2);i++)
								{
								//��8λ����תΪ16λ����
									HMI_Message_Send[i + 24] = (uint16_t)(modbus.RX_Data[i*2]<<8 | modbus.RX_Data[i*2+1]);  
								}
								printf("03���ݲ�ѯ��ɣ�ID:%d",modbus.RX_ID);
								modbus.reflag = 0;		//������Ӧ���
							}
							else
							{
								printf("03���ݲ�ѯʧ�ܣ�ID:%d",modbus.TX_ID);
							}
						}
						break;
  	default: break;
  }
}


void	Modbus_Analysis(uint8_t*	Data,uint8_t	Len)  //modbus���ս���
{
	uint8_t i;
	
	modbus.RX_ID	 = Data[0];
	modbus.RX_Feature = Data[1];
	if(modbus.RX_Feature == 0x03 | modbus.RX_Feature == 0x04 | modbus.RX_Feature == 0x02 | modbus.RX_Feature == 0x01) 
	{
		modbus.RX_DatLen = Data[2];
		for(i=3;i<Len-2;i++)
		{
			modbus.RX_Data[i-3]= Data[i];
		}
	}
	else
	{
		modbus.RX_Address[0] = Data[2];
		modbus.RX_Address[1] = Data[3];
		for(i=4;i<Len-2;i++)
		{
			modbus.RX_Data[i-4]= Data[i];
		}
	}
	modbus.RX_Verify[0] = Data[Len - 2];
	modbus.RX_Verify[1] = Data[Len - 1];
	Modbus_Manage();
}




 

