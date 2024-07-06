/**
  ******************************************************************************
  * @file            bsp_ds18b20.c
  * @author		
  * @date			
  * @version         v1.0
  * @note            DHT11 driver
  ******************************************************************************
  */
 
#include "bsp_ds18b20.h"
#include "bsp_delay.h"
 


#if DS18B20_PLURAL

/**
  * @brief DS18B20 ���ģʽ
  */
static void DS18B20_Mode_OUT_PP(GPIO_TypeDef * Port,uint16_t Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init(Port, &GPIO_InitStruct);
}
 
/**
  * @brief DS18B20 ����ģʽ
  */
static void DS18B20_Mode_IN_NP(GPIO_TypeDef * Port,uint16_t Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
 
	HAL_GPIO_Init(Port, &GPIO_InitStruct);
}
 
/**
  * @brief �������ӻ����͸�λ����
  */
static void DS18B20_Reset(GPIO_TypeDef * Port,uint16_t Pin)
{
	DS18B20_Mode_OUT_PP(Port,Pin);		// �������
	
	//DS18B20_OUT_0;						// �������ٲ��� 480us �ĵ͵�ƽ��λ�ź� 
	HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_RESET);		// �������ٲ��� 480us �ĵ͵�ƽ��λ�ź� 
	bsp_delay_us(750);
	
	//DS18B20_OUT_1;						// �����ڲ�����λ�źź��轫��������
	HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);	
	
	// �ӻ����յ������ĸ�λ�źź󣬻��� 15 ~ 60 us ���������һ����������
	bsp_delay_us(15);					
}
 
/**
  * @brief  ���ӻ����������صĴ�������
  * @return 0���ɹ�		1��ʧ��
	*/
static uint8_t DS18B20_Presence(GPIO_TypeDef * Port,uint16_t Pin)
{
	uint8_t pulse_time = 0;
	
	DS18B20_Mode_IN_NP(Port,Pin);				// ������Ϊ���� 
	
	// �ȴ���������ĵ�������������Ϊһ�� 60 ~ 240 us �ĵ͵�ƽ�ź� 
	// �����������û����������ʱ�����ӻ����յ������ĸ�λ�źź󣬻��� 15 ~ 60 us ���������һ����������
 
	while (HAL_GPIO_ReadPin(Port, Pin) && (pulse_time < 100))		// �ȴ��͵�ƽ��Ӧ����
	{
		pulse_time++;
		bsp_delay_us(1);
	}	
	// ���� 100 us �󣬴������嶼��û�е���
	if (pulse_time >= 100)
	{
		return 1;
	}
	else
	{
		pulse_time = 0;		
	}
		
	// ��Ӧ���壨�͵�ƽ���������Ҵ��ڵ�ʱ�䲻�ܳ��� 240 us 
	while(!(HAL_GPIO_ReadPin(Port, Pin)) && pulse_time < 240)
	{
		pulse_time++;
		bsp_delay_us(1);
	}	
	if(pulse_time >= 240)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
 
 /**
   * @brief  DS18B20 ��ʼ������
   * @reurn  0���ɹ�		1��ʧ��
   */
uint8_t DS18B20_Init(GPIO_TypeDef * Port,uint16_t Pin)
{
	DS18B20_Mode_OUT_PP(Port,Pin);
	//DS18B20_OUT_1;
	HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
	
	DS18B20_Reset(Port,Pin);
	return DS18B20_Presence(Port,Pin);
}
 
/**
  * @brief ��DS18B20��ȡһ��bit
  */
static uint8_t DS18B20_ReadBit(GPIO_TypeDef * Port,uint16_t Pin)
{
	uint8_t dat;
	
	DS18B20_Mode_OUT_PP(Port,Pin);	// �� 0 �Ͷ� 1 ��ʱ������Ҫ���� 60 us 
	
	//DS18B20_OUT_0;					// ��ʱ�����ʼ���������������� > 1us < 15us �ĵ͵�ƽ�ź� 
	HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_RESET);
	
	// ���ʱ��ǳ���Ҫ������Ϊ < 15����Ҫ��γ��ԣ�������ò��������ݻ�ֱ�ӳ���
	// ����������Ϊ 10 11 12������ֵҪ������ʱ������ִ��Ч�ʲ���
	bsp_delay_us(10);		
	
	DS18B20_Mode_IN_NP(Port,Pin);		// ���ó����룬�ͷ����ߣ����ⲿ�������轫�������� 
	
	if (HAL_GPIO_ReadPin(Port, Pin) == 1)
	{
		dat = 1;
	}
	else
	{
		dat = 0;
	}
		
	bsp_delay_us(45);				// �����ʱ�����ο�ʱ��ͼ 
	
	return dat;
}
 
/**
  * @brief �� DS18B20 ��һ���ֽڣ���λ����
  */
static uint8_t DS18B20_ReadByte(GPIO_TypeDef * Port,uint16_t Pin)
{
	uint8_t i, j, dat = 0;	
	
	for(i = 0; i < 8; i++) 
	{
		j = DS18B20_ReadBit(Port,Pin);		
		dat = (dat) | (j << i);
	}
	
	return dat;
}
 
/**
  * @brief дһ���ֽڵ� DS18B20����λ����
  */
static void DS18B20_WriteByte(GPIO_TypeDef * Port,uint16_t Pin,uint8_t dat)
{
	uint8_t i, testb;
	DS18B20_Mode_OUT_PP(Port,Pin);
	
	for( i = 0; i < 8; i++ )
	{
		testb = dat & 0x01;
 
		dat = dat >> 1;		
		
		// д 0 ��д 1 ��ʱ������Ҫ����60us 
		
		if (testb)			// ��ǰλд 1
		{			
			//DS18B20_OUT_0;
			HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_RESET);
			
			bsp_delay_us(5);		// ���ͷ���дʱ���ź�
			
			//DS18B20_OUT_1;			// ��ȡ��ƽʱ�䱣�ָߵ�ƽ
			HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
			bsp_delay_us(65);		
		}		
		else						// ��ǰλд 0			
		{			
			//DS18B20_OUT_0;			// ���ͷ���дʱ���ź�
			HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_RESET);
			bsp_delay_us(70);		// ��ȡ��ƽʱ�䱣�ֵ͵�ƽ
			
			//DS18B20_OUT_1;				
			HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
			bsp_delay_us(2);		// �ָ�ʱ��
		}
	}
}
 
/**
  * @brief  ����ƥ�� DS18B20 ROM
  */
static void DS18B20_SkipRom(GPIO_TypeDef * Port,uint16_t Pin)
{
	DS18B20_Reset(Port, Pin);	   
	
	DS18B20_Presence(Port, Pin);	 
	
	DS18B20_WriteByte(Port,Pin,0XCC);		/* ���� ROM */
}
 
/**
  * @brief  ִ��ƥ�� DS18B20 ROM
  */
static void DS18B20_MatchRom(GPIO_TypeDef * Port,uint16_t Pin)
{
	DS18B20_Reset(Port, Pin);	   
	
	DS18B20_Presence(Port, Pin);	 
	
	DS18B20_WriteByte(Port, Pin,0X55);		/* ƥ�� ROM */
}
 
/**
	* �洢���¶���16 λ�Ĵ�������չ�Ķ����Ʋ�����ʽ
	* ��������12λ�ֱ���ʱ������5������λ��7������λ��4��С��λ
	*
	*         |---------����----------|-----С�� �ֱ��� 1/(2^4)=0.0625----|
	* ���ֽ�  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
	*
	*
	*         |-----����λ��0->��  1->��-------|-----------����-----------|
	* ���ֽ�  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
	*
	* 
	* �¶� = ����λ + ���� + С��*0.0625
	*/
 
/**
  * @brief  ������ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ��
  * @retval �¶�ֵ
  */
float DS18B20_GetTemp_SkipRom(GPIO_TypeDef * Port,uint16_t Pin)
{
	uint8_t tpmsb, tplsb;
	int16_t s_tem;
	
	DS18B20_Init(Port, Pin);
	bsp_delay_us(10);
	float f_tem;
	
	DS18B20_SkipRom(Port, Pin);
	DS18B20_WriteByte(Port, Pin,0X44);				/* ��ʼת�� */
	
	DS18B20_SkipRom(Port, Pin);
  DS18B20_WriteByte(Port, Pin,0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte(Port, Pin);		 
	tpmsb = DS18B20_ReadByte(Port, Pin); 
	
	s_tem = tpmsb << 8;
	s_tem = s_tem | tplsb;
	
	if(s_tem < 0)			/* ���¶� */
	{
		f_tem = (~s_tem + 1) * 0.0625f;	
	}
	else
	{
		f_tem = s_tem * 0.0625f;
	}
		
	return f_tem; 	
}
 
/**
  * @brief  ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ds18b20_id�����ڴ�� DS18B20 ���кŵ�������׵�ַ
  */
void DS18B20_ReadId(GPIO_TypeDef * Port,uint16_t Pin,uint8_t *ds18b20_id)
{
	uint8_t uc;
	
	DS18B20_WriteByte(Port, Pin,0x33);       //��ȡ���к�
	
	for (uc = 0; uc < 8; uc++)
	{
		ds18b20_id[uc] = DS18B20_ReadByte(Port, Pin);
	}
}
 
/**
  * @brief  ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ds18b20_id����� DS18B20 ���кŵ�������׵�ַ
  * @retval �¶�ֵ
  */
float DS18B20_GetTemp_MatchRom(GPIO_TypeDef * Port,uint16_t Pin,uint8_t * ds18b20_id)
{
	uint8_t tpmsb, tplsb, i;
	int16_t s_tem;
	float f_tem;
	
	DS18B20_MatchRom(Port, Pin);            	/* ƥ��ROM */
	
  for(i = 0;i < 8; i++)
	{
		DS18B20_WriteByte(Port, Pin,ds18b20_id[i]);
	}
		
	DS18B20_WriteByte(Port, Pin,0X44);				/* ��ʼת�� */
 
	DS18B20_MatchRom(Port, Pin);            	/* ƥ��ROM */
	
	for(i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(Port, Pin,ds18b20_id[i]);	
	}
		
	DS18B20_WriteByte(Port, Pin,0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte(Port, Pin);		 
	tpmsb = DS18B20_ReadByte(Port, Pin); 
	
	s_tem = tpmsb << 8;
	s_tem = s_tem | tplsb;
	
	if (s_tem < 0)									/* ���¶� */
	{
		f_tem = (~s_tem + 1) * 0.0625f;	
	}
	else
	{
		f_tem = s_tem * 0.0625f;
	}
		
	return f_tem; 		
}
 
// ���Գ���
 
#if 0
 
uint8_t uc, ucDs18b20Id[8];
	
while (DS18B20_Init())
{
	printf("\r\n no ds18b20 exit \r\n");
}
 
printf("\r\n ds18b20 exit \r\n");
 
DS18B20_ReadId(ucDs18b20Id);           	// ��ȡ DS18B20 �����к�
printf("\r\nDS18B20�����к��ǣ� 0x");
 
for (uc = 0; uc < 8; uc++)          		// ��ӡ DS18B20 �����к�
{
	printf("%.2x", ucDs18b20Id[uc]);
}
 
while (1)
{
	printf("\r\n�¶ȣ� %.1f\r\n", DS18B20_GetTemp_SkipRom());	
 
	HAL_Delay(1000);		/* 1s ��ȡһ���¶�ֵ */
}
 
#endif






#else









/**
  * @brief DS18B20 ���ģʽ
  */
static void DS18B20_Mode_OUT_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = BSP_DS18B20_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(BSP_DS18B20_PORT, &GPIO_InitStruct);
}
 
/**
  * @brief DS18B20 ����ģʽ
  */
static void DS18B20_Mode_IN_NP(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = BSP_DS18B20_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
 
	HAL_GPIO_Init(BSP_DS18B20_PORT, &GPIO_InitStruct);
}
 
/**
  * @brief �������ӻ����͸�λ����
  */
static void DS18B20_Reset(void)
{
	DS18B20_Mode_OUT_PP();		// �������
	
	DS18B20_OUT_0;						// �������ٲ��� 480us �ĵ͵�ƽ��λ�ź� 
	bsp_delay_us(750);
	
	DS18B20_OUT_1;						// �����ڲ�����λ�źź��轫�������� 
	
	// �ӻ����յ������ĸ�λ�źź󣬻��� 15 ~ 60 us ���������һ����������
	bsp_delay_us(15);					
}
 
/**
  * @brief  ���ӻ����������صĴ�������
  * @return 0���ɹ�		1��ʧ��
	*/
static uint8_t DS18B20_Presence(void)
{
	uint8_t pulse_time = 0;
	
	DS18B20_Mode_IN_NP();				// ������Ϊ���� 
	
	// �ȴ���������ĵ�������������Ϊһ�� 60 ~ 240 us �ĵ͵�ƽ�ź� 
	// �����������û����������ʱ�����ӻ����յ������ĸ�λ�źź󣬻��� 15 ~ 60 us ���������һ����������
 
	while (DS18B20_IN && (pulse_time < 100))		// �ȴ��͵�ƽ��Ӧ����
	{
		pulse_time++;
		bsp_delay_us(1);
	}	
	// ���� 100 us �󣬴������嶼��û�е���
	if (pulse_time >= 100)
	{
		return 1;
	}
	else
	{
		pulse_time = 0;		
	}
		
	// ��Ӧ���壨�͵�ƽ���������Ҵ��ڵ�ʱ�䲻�ܳ��� 240 us 
	while(!(DS18B20_IN) && pulse_time < 240)
	{
		pulse_time++;
		bsp_delay_us(1);
	}	
	if(pulse_time >= 240)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
 
 /**
   * @brief  DS18B20 ��ʼ������
   * @reurn  0���ɹ�		1��ʧ��
   */
uint8_t DS18B20_Init(void)
{
	DS18B20_Mode_OUT_PP();
	DS18B20_OUT_1;
	
	DS18B20_Reset();
	return DS18B20_Presence();
}
 
/**
  * @brief ��DS18B20��ȡһ��bit
  */
static uint8_t DS18B20_ReadBit(void)
{
	uint8_t dat;
	
	DS18B20_Mode_OUT_PP();	// �� 0 �Ͷ� 1 ��ʱ������Ҫ���� 60 us 
	
	DS18B20_OUT_0;					// ��ʱ�����ʼ���������������� > 1us < 15us �ĵ͵�ƽ�ź� 
	
	// ���ʱ��ǳ���Ҫ������Ϊ < 15����Ҫ��γ��ԣ�������ò��������ݻ�ֱ�ӳ���
	// ����������Ϊ 10 11 12������ֵҪ������ʱ������ִ��Ч�ʲ���
	bsp_delay_us(10);		
	
	DS18B20_Mode_IN_NP();		// ���ó����룬�ͷ����ߣ����ⲿ�������轫�������� 
	
	if (DS18B20_IN == 1)
	{
		dat = 1;
	}
	else
	{
		dat = 0;
	}
		
	bsp_delay_us(45);				// �����ʱ�����ο�ʱ��ͼ 
	
	return dat;
}
 
/**
  * @brief �� DS18B20 ��һ���ֽڣ���λ����
  */
static uint8_t DS18B20_ReadByte(void)
{
	uint8_t i, j, dat = 0;	
	
	for(i = 0; i < 8; i++) 
	{
		j = DS18B20_ReadBit();		
		dat = (dat) | (j << i);
	}
	
	return dat;
}
 
/**
  * @brief дһ���ֽڵ� DS18B20����λ����
  */
static void DS18B20_WriteByte(uint8_t dat)
{
	uint8_t i, testb;
	DS18B20_Mode_OUT_PP();
	
	for( i = 0; i < 8; i++ )
	{
		testb = dat & 0x01;
 
		dat = dat >> 1;		
		
		// д 0 ��д 1 ��ʱ������Ҫ����60us 
		
		if (testb)			// ��ǰλд 1
		{			
			DS18B20_OUT_0;
			
			bsp_delay_us(5);		// ���ͷ���дʱ���ź�
			
			DS18B20_OUT_1;			// ��ȡ��ƽʱ�䱣�ָߵ�ƽ
			bsp_delay_us(65);		
		}		
		else						// ��ǰλд 0			
		{			
			DS18B20_OUT_0;			// ���ͷ���дʱ���ź�
			bsp_delay_us(70);		// ��ȡ��ƽʱ�䱣�ֵ͵�ƽ
			
			DS18B20_OUT_1;				
			bsp_delay_us(2);		// �ָ�ʱ��
		}
	}
}
 
/**
  * @brief  ����ƥ�� DS18B20 ROM
  */
static void DS18B20_SkipRom(void)
{
	DS18B20_Reset();	   
	
	DS18B20_Presence();	 
	
	DS18B20_WriteByte(0XCC);		/* ���� ROM */
}
 
/**
  * @brief  ִ��ƥ�� DS18B20 ROM
  */
static void DS18B20_MatchRom(void)
{
	DS18B20_Reset();	   
	
	DS18B20_Presence();	 
	
	DS18B20_WriteByte(0X55);		/* ƥ�� ROM */
}
 
/**
	* �洢���¶���16 λ�Ĵ�������չ�Ķ����Ʋ�����ʽ
	* ��������12λ�ֱ���ʱ������5������λ��7������λ��4��С��λ
	*
	*         |---------����----------|-----С�� �ֱ��� 1/(2^4)=0.0625----|
	* ���ֽ�  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
	*
	*
	*         |-----����λ��0->��  1->��-------|-----------����-----------|
	* ���ֽ�  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
	*
	* 
	* �¶� = ����λ + ���� + С��*0.0625
	*/
 
/**
  * @brief  ������ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ��
  * @retval �¶�ֵ
  */
float DS18B20_GetTemp_SkipRom(void)
{
	uint8_t tpmsb, tplsb;
	int16_t s_tem;
	float f_tem;
	
	DS18B20_SkipRom();
	DS18B20_WriteByte(0X44);				/* ��ʼת�� */
	
	DS18B20_SkipRom();
  DS18B20_WriteByte(0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	s_tem = tpmsb << 8;
	s_tem = s_tem | tplsb;
	
	if(s_tem < 0)			/* ���¶� */
	{
		f_tem = (~s_tem + 1) * 0.0625f;	
	}
	else
	{
		f_tem = s_tem * 0.0625f;
	}
		
	return f_tem; 	
}
 
/**
  * @brief  ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ds18b20_id�����ڴ�� DS18B20 ���кŵ�������׵�ַ
  */
void DS18B20_ReadId(uint8_t *ds18b20_id)
{
	uint8_t uc;
	
	DS18B20_WriteByte(0x33);       //��ȡ���к�
	
	for (uc = 0; uc < 8; uc++)
	{
		ds18b20_id[uc] = DS18B20_ReadByte();
	}
}
 
/**
  * @brief  ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * @param  ds18b20_id����� DS18B20 ���кŵ�������׵�ַ
  * @retval �¶�ֵ
  */
float DS18B20_GetTemp_MatchRom(uint8_t * ds18b20_id)
{
	uint8_t tpmsb, tplsb, i;
	int16_t s_tem;
	float f_tem;
	
	DS18B20_MatchRom();            	/* ƥ��ROM */
	
  for(i = 0;i < 8; i++)
	{
		DS18B20_WriteByte(ds18b20_id[i]);
	}
		
	DS18B20_WriteByte(0X44);				/* ��ʼת�� */
 
	DS18B20_MatchRom();            	/* ƥ��ROM */
	
	for(i = 0; i < 8; i++)
	{
		DS18B20_WriteByte(ds18b20_id[i]);	
	}
		
	DS18B20_WriteByte(0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	s_tem = tpmsb << 8;
	s_tem = s_tem | tplsb;
	
	if (s_tem < 0)									/* ���¶� */
	{
		f_tem = (~s_tem + 1) * 0.0625f;	
	}
	else
	{
		f_tem = s_tem * 0.0625f;
	}
		
	return f_tem; 		
}
 
// ���Գ���
 
#if 0
 
uint8_t uc, ucDs18b20Id[8];
	
while (DS18B20_Init())
{
	printf("\r\n no ds18b20 exit \r\n");
}
 
printf("\r\n ds18b20 exit \r\n");
 
DS18B20_ReadId(ucDs18b20Id);           	// ��ȡ DS18B20 �����к�
printf("\r\nDS18B20�����к��ǣ� 0x");
 
for (uc = 0; uc < 8; uc++)          		// ��ӡ DS18B20 �����к�
{
	printf("%.2x", ucDs18b20Id[uc]);
}
 
while (1)
{
	printf("\r\n�¶ȣ� %.1f\r\n", DS18B20_GetTemp_SkipRom());	
 
	HAL_Delay(1000);		/* 1s ��ȡһ���¶�ֵ */
}
 
#endif

#endif
