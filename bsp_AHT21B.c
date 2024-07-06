#include "bsp_AHT21B.h"


//#include "StdLibs.h"
//#include "SysLibs.h"
//#include "AppLibs.h"


#include "stdio.h"
#include "spi.h"

float  Temperature = 0.0;

float  Humidity = 0.0;



uint8_t AHT20_Read_Status( void ) //读取AHT20的状态寄存器
{
    uint8_t Byte_first;

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, &Byte_first, 1, AHT20_IIC_BUS_WAIT );

    return Byte_first;
}

uint8_t AHT20_Read_Cal_Enable( void ) //查询cal enable位有没有使能
{
    uint8_t val = 0;//ret = 0,
    val = AHT20_Read_Status();
    if( ( val & 0x68 ) == 0x08 )
        return 1;
    else  return 0;
}


void Aht20_Reset( void ) //向AHT20发送软件复位命令
{
    uint8_t cmd_buf=0XBA;
    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, &cmd_buf, 1, AHT20_IIC_BUS_WAIT );  
}

void AHT20_SendAC( void ) //向AHT20发送AC命令
{

    uint8_t cmd_buf[3] = {0xac, 0x33, 0x00};
    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
}

//CRC校验类型：CRC8/MAXIM
//多项式：X8+X5+X4+1
//Poly：0011 0001  0x31
//高位放到后面就变成 1000 1100 0x8c
//C现实代码：
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

void AHT20_Read_CTdata( uint32_t *ct ) //没有CRC校验，直接读取AHT20的温度和湿度数据
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
    AHT20_SendAC();//向AHT10发送AC命令

    HAL_Delay( 80 ); //延时80ms左右

    cnt = 0;
    while( ( ( AHT20_Read_Status() & 0x80 ) == 0x80 ) ) //直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
    {
        HAL_Delay( 2 );
        if( cnt++ >= 100 )
        {
            break;
        }
    }

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 6, AHT20_IIC_BUS_WAIT );

    Byte_1th = read_buf[0]; //状态字，查询到状态为0x98,表示为忙状态，bit[7]为1；状态为0x1C，或者0x0C，或者0x08表示为空闲状态，bit[7]为0
    Byte_2th = read_buf[1]; //湿度
    Byte_3th = read_buf[2]; //湿度
    Byte_4th = read_buf[3]; //湿度/温度
    Byte_5th = read_buf[4]; //温度
    Byte_6th = read_buf[5]; //温度



    RetuData = ( RetuData | Byte_2th ) << 8;
    RetuData = ( RetuData | Byte_3th ) << 8;
    RetuData = ( RetuData | Byte_4th );
    RetuData = RetuData >> 4;
    ct[0] = RetuData;//湿度
    RetuData = 0;
    RetuData = ( RetuData | Byte_4th ) << 8;
    RetuData = ( RetuData | Byte_5th ) << 8;
    RetuData = ( RetuData | Byte_6th );
    RetuData = RetuData & 0xfffff;
    ct[1] = RetuData; //温度

}


//void AHT20_Read_CTdata_crc(uint32_t *ct) //CRC校验后，读取AHT20的温度和湿度数据
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
//	 uint8_t  CTDATA[6]={0};//用于CRC传递数组
//
//
//    uint8_t  read_buf[7];
//
//
//	AHT20_SendAC();//向AHT10发送AC命令
//	HAL_Delay(80);//延时80ms左右
//    cnt = 0;
//	while(((AHT20_Read_Status()&0x80)==0x80))//直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
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
// 	CTDATA[0]=Byte_1th = read_buf[0];//状态字，查询到状态为0x98,表示为忙状态，bit[7]为1；状态为0x1C，或者0x0C，或者0x08表示为空闲状态，bit[7]为0
//	CTDATA[1]=Byte_2th = read_buf[1];//湿度
//	CTDATA[2]=Byte_3th = read_buf[2];//湿度
//	CTDATA[3]=Byte_4th = read_buf[3];//湿度/温度
//	CTDATA[4]=Byte_5th = read_buf[4];//温度
//	CTDATA[5]=Byte_6th = read_buf[5];//温度
//	Byte_7th = read_buf[6];//CRC数据
//
//	if(Calc_CRC8(CTDATA,6)==Byte_7th)
//	{
//	RetuData = (RetuData|Byte_2th)<<8;
//	RetuData = (RetuData|Byte_3th)<<8;
//	RetuData = (RetuData|Byte_4th);
//	RetuData =RetuData >>4;
//	ct[0] = RetuData;//湿度
//	RetuData = 0;
//	RetuData = (RetuData|Byte_4th)<<8;
//	RetuData = (RetuData|Byte_5th)<<8;
//	RetuData = (RetuData|Byte_6th);
//	RetuData = RetuData&0xfffff;
//	ct[1] =RetuData; //温度
//
//	}
//	else
//	{
//		ct[0]=0x00;
//		ct[1]=0x00;//校验错误返回值，客户可以根据自己需要更改
//	}//CRC数据
//}


void AHT20_Init( void ) //初始化AHT20
{

    uint8_t cmd_buf[3];
    cmd_buf[0] = 0xa8;
    cmd_buf[1] = 0x00;
    cmd_buf[2] = 0x00;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
    HAL_Delay( 10 ); //延时10ms左右

    cmd_buf[0] = 0xbe;
    cmd_buf[1] = 0x08;
    cmd_buf[2] = 0x00;

    HAL_I2C_Master_Transmit( &AHT20_IIC, AHT20_IIC_ADDR, cmd_buf, 3, AHT20_IIC_BUS_WAIT );
    HAL_Delay( 10 ); //延时10ms左右
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

    HAL_Delay( 5 ); //延时5ms左右
    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 3, AHT20_IIC_BUS_WAIT );
    //Byte_first = read_buf[0];
    Byte_second = read_buf[1];
    Byte_third = read_buf[2];

    HAL_Delay( 10 ); //延时10ms左右

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
//把函数拆分出来，因为里面延时 要去掉不必要的延时
void Aht20_Task_Start( void )
{
    AHT20_SendAC();//向AHT10发送AC命令
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

    while( ( ( AHT20_Read_Status() & 0x80 ) == 0x80 ) ) //直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
    {
        HAL_Delay( 1 );
        if( cnt++ >= 100 )
        {
            return ; //不读取此次结果
        }
    }
    

    HAL_I2C_Master_Receive( &AHT20_IIC, AHT20_IIC_ADDR, read_buf, 6, AHT20_IIC_BUS_WAIT );

    Byte_1th = read_buf[0]; //状态字，查询到状态为0x98,表示为忙状态，bit[7]为1；状态为0x1C，或者0x0C，或者0x08表示为空闲状态，bit[7]为0
    Byte_2th = read_buf[1]; //湿度
    Byte_3th = read_buf[2]; //湿度
    Byte_4th = read_buf[3]; //湿度/温度
    Byte_5th = read_buf[4]; //温度
    Byte_6th = read_buf[5]; //温度


    RetuData = ( RetuData | Byte_2th ) << 8;
    RetuData = ( RetuData | Byte_3th ) << 8;
    RetuData = ( RetuData | Byte_4th );
    RetuData = RetuData >> 4;
    CT_data[0] = RetuData;//湿度
    RetuData = 0;
    RetuData = ( RetuData | Byte_4th ) << 8;
    RetuData = ( RetuData | Byte_5th ) << 8;
    RetuData = ( RetuData | Byte_6th );
    RetuData = RetuData & 0xfffff;
    CT_data[1] = RetuData; //温度


//    c1 = CT_data[0] * 100 * 10 / 1024 / 1024; //计算得到湿度值c1（放大了10倍）
//    t1 = CT_data[1] * 200 * 10 / 1024 / 1024 - 500; //计算得到温度值t1（放大了10倍）
//
//    Humidity = c1;
//    Humidity = Humidity / 10;
//    Temperature = t1;
//    Temperature = Temperature / 10;
 

    Humidity =(float) CT_data[0]*100/1024/1024;  //计算得到湿度值c1
    Temperature =(float) CT_data[1]*200/1024/1024-50;//计算得到温度值t1

    printf ("Humidity:%.1f, Temperature:%.1f\r\n", Humidity, Temperature );
    //DBG_PRINTF( string_temp );

}














