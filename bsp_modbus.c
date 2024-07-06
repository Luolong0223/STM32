#include "bsp_modbus.h"
#include "stdio.h"
#include "usart.h"
//#include "tim.h"
#include <stdint.h>

MODBUS modbus;
#define MessageLen 72   //写入HMI的数据数量
uint16_t HMI_Message_Send[MessageLen];
uint8_t  HMI_Message_Send_8b[MessageLen*2];
uint16_t HMI_Message_Read[10];
uint8_t  HMI_Message_Read_8b[20];

// CRC计算函数
uint16_t ModbusCRC(uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF; // CRC的初始值
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos]; // 将数据字节与CRC寄存器异或
        for (int i = 8; i != 0; i--) { // 处理每个位
            if ((crc & 0x0001) != 0) { // 如果CRC寄存器的最低位为1
                crc >>= 1; // 右移一位
                crc ^= 0xA001; // 与多项式A001（1010 0000 0000 0001）异或
            } else {
                crc >>= 1; // 如果最低位为0，只需右移一位
            }
        }
    }
    // 返回计算得到的CRC值
    return crc;
}
// 验证CRC的函数
int VerifyModbusCRC(uint8_t *data, int len) 
{
    if (len < 2) {
        // 数据长度至少包括两个字节（CRC）
        return 0;
    }

		// 从数据中提取CRC值
    uint16_t receivedCRC = (uint16_t)(data[len - 1] << 8 | data[len - 2]);
		
    // 计算CRC
    uint16_t calculatedCRC = ModbusCRC(data, len - 2);

    // 比较接收到的CRC和计算得到的CRC

		 return (receivedCRC == calculatedCRC);
}

void	HMI_Send(uint8_t id)  //HMI通讯发送帧0x10
{
	uint16_t crc;
	uint8_t T_DA[200];
	uint8_t i;
	for (int i = 0; i < sizeof(HMI_Message_Send); i++) 
	{
     HMI_Message_Send_8b[2 * i] = (unsigned char)(HMI_Message_Send[i] >> 8);     // 获取高8位
     HMI_Message_Send_8b[2 * i + 1] = (unsigned char)(HMI_Message_Send[i] & 0xFF); // 获取低8位
  }
	modbus.TX_ID	 = id;
	modbus.TX_Feature = 0x10;
	modbus.TX_Address[0] = 0x00;							//起始地址
	modbus.TX_Address[1] = 0x00;
	modbus.TX_DatLen[0] = (sizeof(HMI_Message_Send_8b)/2)/256;		//数据长度
	modbus.TX_DatLen[1] = (sizeof(HMI_Message_Send_8b)/2)%256;
	modbus.TX_DataCont = sizeof(HMI_Message_Send_8b)%256;				//数据字节个数
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
	modbus.reflag = 1;		//HMI发送数据等待响应，相应完成 = 0
	HAL_UART_Transmit_DMA(&huart2,modbus.TX_Buf,sizeof(HMI_Message_Send_8b)+9);
	//HAL_UART_Transmit(&huart1,modbus.TX_Buf,sizeof(HMI_Message_Send_8b)+9,0xffff);	//UART1发送MODBUS数据  调试
}


void	HMI_Read(uint8_t id)      //读取HMI输出类容
{
	uint16_t crc;
	uint8_t T_DA[6];
	uint8_t i;

	modbus.TX_ID	 = id;
	modbus.TX_Feature = 0x04;
	modbus.TX_Address[0] = 0x00;							//起始地址
	modbus.TX_Address[1] = 0x00;
	modbus.TX_DatLen[0] = (sizeof(HMI_Message_Read_8b)/2)/256;		//数据长度
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


void Modbus_03Read(uint8_t ID,uint16_t START_ADDRESS,uint16_t DataLen) //03查询码
{
	uint16_t crc;
	uint8_t T_DA[6];
	uint8_t i;
	
	modbus.TX_ID 				 = ID;
	modbus.TX_Feature 	 = 0x03;
	modbus.TX_Address[0] = (unsigned char) (START_ADDRESS >> 8); 		// 获取高8位
	modbus.TX_Address[1] = (unsigned char) (START_ADDRESS & 0xFF);   // 获取低8位
	modbus.TX_DatLen[0]  = (unsigned char) (DataLen >> 8); 		// 获取高8位
	modbus.TX_DatLen[1]  = (unsigned char) (DataLen & 0xFF);   // 获取低8位
	
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


void Modbus_Manage(void)	//MODBUS接收数据处理
{
	switch (modbus.reflag)
  {
  	case 1:
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//HMI返回CRC验证成功
							if(modbus.RX_Feature == 0x10) //判断返回功能码
							{
								printf("HMI数据推送完成！");
								modbus.reflag = 0;		//HMI发送响应完成
							}
							else
							{
								//HAL_UART_Transmit(&huart1,(uint8_t*)"HMI推送数据错误！",17,0xffff);
								printf("HMI数据推送完成！");
							}
						}
						break;
  	case 2:
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//HMI返回CRC验证成功
							if(modbus.RX_Feature == 0x04) //判断返回功能码
							{
								
								for (int i = 0; i < sizeof(HMI_Message_Read); i++) 
								{
									//将接收8位数据转换成16位		
									HMI_Message_Read[i] = (uint8_t)modbus.RX_Data[2*i] << 8 | (uint8_t)modbus.RX_Data[2*i+1];  			
								}
								
								printf("HMI数据查询完成！");
								modbus.reflag = 0;		//HMI发送响应完成
							}
							else
							{
								printf("HMI数据查询失败！");
							}
						}
						break;
			
		case 3: 
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//返回CRC验证成功
							if(modbus.RX_Feature == 0x03) //判断返回功能码
							{
								//将8位数据转为16位数据
								HMI_Message_Send[modbus.RX_ID-1] = (uint16_t)(modbus.RX_Data[0]<<8 | modbus.RX_Data[1]);  
								printf("03数据查询完成！ID:%d",modbus.RX_ID);
								modbus.reflag = 0;		//发送响应完成
							}
							else
							{
								printf("03数据查询失败！ID:%d",modbus.TX_ID);
							}
						}
						break;
		case 4: 
						if(modbus.CRC_OK ==1)
						{
							modbus.CRC_OK = 0;		//返回CRC验证成功
							if(modbus.RX_Feature == 0x03) //判断返回功能码
							{
								for(int i=0;i<(modbus.RX_DatLen/2);i++)
								{
								//将8位数据转为16位数据
									HMI_Message_Send[i + 24] = (uint16_t)(modbus.RX_Data[i*2]<<8 | modbus.RX_Data[i*2+1]);  
								}
								printf("03数据查询完成！ID:%d",modbus.RX_ID);
								modbus.reflag = 0;		//发送响应完成
							}
							else
							{
								printf("03数据查询失败！ID:%d",modbus.TX_ID);
							}
						}
						break;
  	default: break;
  }
}


void	Modbus_Analysis(uint8_t*	Data,uint8_t	Len)  //modbus接收解析
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




 

