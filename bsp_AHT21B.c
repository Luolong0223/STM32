#include "bsp_AHT21B.h"


//#include "StdLibs.h"
//#include "SysLibs.h"
//#include "AppLibs.h"


#include "stdio.h"
#include "spi.h"

float  Temperature = 0.0;

float  Humidity = 0.0;



uint8_t AHT20_Read_Status( void ) //��ȡAHT20��״̬�Ĵ���
{
    uint8_t Byte_first;

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, &Byte_first, 1, AHT20_IIC_BUS_WAIT );

    return Byte_first;
}

uint8_t AHT20_Read_Cal_Enable( void ) //��ѯcal enableλ��û��ʹ��
{
    uint8_t val = 0;//ret = 0,
    val = AHT20_Read_Status();
    if( ( val & 0x68 ) == 0x08 )
        return 1;
    else  return 0;
}


void Aht20_Reset( void ) //��AHT20���������λ����
{
    uint8_t cmd_buf=0XBA;
    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, &cmd_buf, 1, AHT20_IIC_BUS_WAIT );  
}

void AHT20_SendAC( void ) //��AHT20����AC����
{

    uint8_t cmd_buf[3] = {0xac, 0x33, 0x00};
    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
}

//CRCУ�����ͣ�CRC8/MAXIM
//����ʽ��X8+X5+X4+1
//Poly��0011 0001  0x31
//��λ�ŵ�����ͱ�� 1000 1100 0x8c
//C��ʵ���룺
uint8_t Calc_CRC8( uint8_t *message, uint8_t Num )
{
    uint8_t i;
    uint8_t byte;
    uint8_t crc = 0xFF;
    for( byte = 0; byte < Num; byte++ )
    {
        crc ^= ( message[byte] );
        for( i = 8; i > 0; --i )
        {
            if( crc & 0x80 ) crc = ( crc << 1 ) ^ 0x31;
            else crc = ( crc << 1 );
        }
    }
    return crc;
}

void AHT20_Read_CTdata( uint32_t *ct ) //û��CRCУ�飬ֱ�Ӷ�ȡAHT20���¶Ⱥ�ʪ������
{
    volatile uint8_t  Byte_1th = 0;
    volatile uint8_t  Byte_2th = 0;
    volatile uint8_t  Byte_3th = 0;
    volatile uint8_t  Byte_4th = 0;
    volatile uint8_t  Byte_5th = 0;
    volatile uint8_t  Byte_6th = 0;


    uint8_t  read_buf[6];

    uint32_t RetuData = 0;
    uint16_t cnt = 0;
    AHT20_SendAC();//��AHT10����AC����

    HAL_Delay( 80 ); //��ʱ80ms����

    cnt = 0;
    while( ( ( AHT20_Read_Status() & 0x80 ) == 0x80 ) ) //ֱ��״̬bit[7]Ϊ0����ʾΪ����״̬����Ϊ1����ʾæ״̬
    {
        HAL_Delay( 2 );
        if( cnt++ >= 100 )
        {
            break;
        }
    }

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 6, AHT20_IIC_BUS_WAIT );

    Byte_1th = read_buf[0]; //״̬�֣���ѯ��״̬Ϊ0x98,��ʾΪæ״̬��bit[7]Ϊ1��״̬Ϊ0x1C������0x0C������0x08��ʾΪ����״̬��bit[7]Ϊ0
    Byte_2th = read_buf[1]; //ʪ��
    Byte_3th = read_buf[2]; //ʪ��
    Byte_4th = read_buf[3]; //ʪ��/�¶�
    Byte_5th = read_buf[4]; //�¶�
    Byte_6th = read_buf[5]; //�¶�



    RetuData = ( RetuData | Byte_2th ) << 8;
    RetuData = ( RetuData | Byte_3th ) << 8;
    RetuData = ( RetuData | Byte_4th );
    RetuData = RetuData >> 4;
    ct[0] = RetuData;//ʪ��
    RetuData = 0;
    RetuData = ( RetuData | Byte_4th ) << 8;
    RetuData = ( RetuData | Byte_5th ) << 8;
    RetuData = ( RetuData | Byte_6th );
    RetuData = RetuData & 0xfffff;
    ct[1] = RetuData; //�¶�

}


//void AHT20_Read_CTdata_crc(uint32_t *ct) //CRCУ��󣬶�ȡAHT20���¶Ⱥ�ʪ������
//{
//	volatile uint8_t  Byte_1th=0;
//	volatile uint8_t  Byte_2th=0;
//	volatile uint8_t  Byte_3th=0;
//	volatile uint8_t  Byte_4th=0;
//	volatile uint8_t  Byte_5th=0;
//	volatile uint8_t  Byte_6th=0;
//	volatile uint8_t  Byte_7th=0;
//	 uint32_t RetuData = 0;
//	 uint16_t cnt = 0;
//	// uint8_t  CRCDATA=0;
//	 uint8_t  CTDATA[6]={0};//����CRC��������
//
//
//    uint8_t  read_buf[7];
//
//
//	AHT20_SendAC();//��AHT10����AC����
//	HAL_Delay(80);//��ʱ80ms����
//    cnt = 0;
//	while(((AHT20_Read_Status()&0x80)==0x80))//ֱ��״̬bit[7]Ϊ0����ʾΪ����״̬����Ϊ1����ʾæ״̬
//	{
//		HAL_Delay(2);
//		if(cnt++>=100)
//		{
//		 break;
//		}
//	}
//
//
//    HAL_I2C_Master_Receive(&AHT20_IIC, AHT20_IIC_ADDR, read_buf,7,AHT20_IIC_BUS_WAIT);
//
//
// 	CTDATA[0]=Byte_1th = read_buf[0];//״̬�֣���ѯ��״̬Ϊ0x98,��ʾΪæ״̬��bit[7]Ϊ1��״̬Ϊ0x1C������0x0C������0x08��ʾΪ����״̬��bit[7]Ϊ0
//	CTDATA[1]=Byte_2th = read_buf[1];//ʪ��
//	CTDATA[2]=Byte_3th = read_buf[2];//ʪ��
//	CTDATA[3]=Byte_4th = read_buf[3];//ʪ��/�¶�
//	CTDATA[4]=Byte_5th = read_buf[4];//�¶�
//	CTDATA[5]=Byte_6th = read_buf[5];//�¶�
//	Byte_7th = read_buf[6];//CRC����
//
//	if(Calc_CRC8(CTDATA,6)==Byte_7th)
//	{
//	RetuData = (RetuData|Byte_2th)<<8;
//	RetuData = (RetuData|Byte_3th)<<8;
//	RetuData = (RetuData|Byte_4th);
//	RetuData =RetuData >>4;
//	ct[0] = RetuData;//ʪ��
//	RetuData = 0;
//	RetuData = (RetuData|Byte_4th)<<8;
//	RetuData = (RetuData|Byte_5th)<<8;
//	RetuData = (RetuData|Byte_6th);
//	RetuData = RetuData&0xfffff;
//	ct[1] =RetuData; //�¶�
//
//	}
//	else
//	{
//		ct[0]=0x00;
//		ct[1]=0x00;//У����󷵻�ֵ���ͻ����Ը����Լ���Ҫ����
//	}//CRC����
//}


void AHT20_Init( void ) //��ʼ��AHT20
{

    uint8_t cmd_buf[3];
    cmd_buf[0] = 0xa8;
    cmd_buf[1] = 0x00;
    cmd_buf[2] = 0x00;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
    HAL_Delay( 10 ); //��ʱ10ms����

    cmd_buf[0] = 0xbe;
    cmd_buf[1] = 0x08;
    cmd_buf[2] = 0x00;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
    HAL_Delay( 10 ); //��ʱ10ms����
}
void JH_Reset_REG( uint8_t addr )
{

    uint8_t Byte_second, Byte_third;

    uint8_t cmd_buf[3];
    uint8_t read_buf[3];

    cmd_buf[0] = addr;
    cmd_buf[1] = 0x00;
    cmd_buf[2] = 0x00;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );

    HAL_Delay( 5 ); //��ʱ5ms����
    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 3, AHT20_IIC_BUS_WAIT );
    //Byte_first = read_buf[0];
    Byte_second = read_buf[1];
    Byte_third = read_buf[2];

    HAL_Delay( 10 ); //��ʱ10ms����

    cmd_buf[0] = 0xB0 | addr;
    cmd_buf[1] = Byte_second;
    cmd_buf[2] = Byte_third;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );

    Byte_second = 0x00;
    Byte_third = 0x00;
}

void AHT20_Start_Init( void )
{
    JH_Reset_REG( 0x1b );
    JH_Reset_REG( 0x1c );
    JH_Reset_REG( 0x1e );
}

//AHT20_Read_CTdata(CT_data);
//AHT20_Read_CTdata_crc(CT_data);
//�Ѻ�����ֳ�������Ϊ������ʱ Ҫȥ������Ҫ����ʱ
void Aht20_Task_Start( void )
{
    AHT20_SendAC();//��AHT10����AC����
}


void Aht20_Task_Read(void)
{
    uint32_t CT_data[2]={0,0};

		
    Aht20_Reset();   
		HAL_Delay(10);
    Aht20_Task_Start();  
		HAL_Delay(10);
		
    volatile uint8_t  Byte_1th = 0;
    volatile uint8_t  Byte_2th = 0;
    volatile uint8_t  Byte_3th = 0;
    volatile uint8_t  Byte_4th = 0;
    volatile uint8_t  Byte_5th = 0;
    volatile uint8_t  Byte_6th = 0;

    uint8_t  read_buf[6];
    uint32_t RetuData = 0;
    

    uint16_t cnt = 0;

    while( ( ( AHT20_Read_Status() & 0x80 ) == 0x80 ) ) //ֱ��״̬bit[7]Ϊ0����ʾΪ����״̬����Ϊ1����ʾæ״̬
    {
        HAL_Delay( 1 );
        if( cnt++ >= 100 )
        {
            return ; //����ȡ�˴ν��
        }
    }
    

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 6, AHT20_IIC_BUS_WAIT );

    Byte_1th = read_buf[0]; //״̬�֣���ѯ��״̬Ϊ0x98,��ʾΪæ״̬��bit[7]Ϊ1��״̬Ϊ0x1C������0x0C������0x08��ʾΪ����״̬��bit[7]Ϊ0
    Byte_2th = read_buf[1]; //ʪ��
    Byte_3th = read_buf[2]; //ʪ��
    Byte_4th = read_buf[3]; //ʪ��/�¶�
    Byte_5th = read_buf[4]; //�¶�
    Byte_6th = read_buf[5]; //�¶�


    RetuData = ( RetuData | Byte_2th ) << 8;
    RetuData = ( RetuData | Byte_3th ) << 8;
    RetuData = ( RetuData | Byte_4th );
    RetuData = RetuData >> 4;
    CT_data[0] = RetuData;//ʪ��
    RetuData = 0;
    RetuData = ( RetuData | Byte_4th ) << 8;
    RetuData = ( RetuData | Byte_5th ) << 8;
    RetuData = ( RetuData | Byte_6th );
    RetuData = RetuData & 0xfffff;
    CT_data[1] = RetuData; //�¶�


//    c1 = CT_data[0] * 100 * 10 / 1024 / 1024; //����õ�ʪ��ֵc1���Ŵ���10����
//    t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; //����õ��¶�ֵt1���Ŵ���10����
//
//    Humidity = c1;
//    Humidity = Humidity / 10;
//    Temperature = t1;
//    Temperature = Temperature / 10;
 

    Humidity =(float) CT_data[0]*100/1024/1024;  //����õ�ʪ��ֵc1
    Temperature =(float) CT_data[1]*200/1024/1024-50;//����õ��¶�ֵt1

    printf ("Humidity:%.1f, Temperature:%.1f\r\n", Humidity, Temperature );
    //DBG_PRINTF( string_temp );

}














