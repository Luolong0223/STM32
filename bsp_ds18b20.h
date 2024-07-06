/**
  ******************************************************************************
  * @file			bsp_ds18b20.h
  * @author		
  * @date			
  * @version	    v1.0
  * @note			DS18B20 driver
  ******************************************************************************
  */
 
#ifndef __BSP_DS18B20_H
#define __BSP_DS18B20_H
 
#include "main.h"

#define DS18B20_PLURAL 1		// DS18B20_PLURAL = 1  多个DS18B20，DS18B20_PLURAL = 0  单个使用

#if DS18B20_PLURAL
 


uint8_t DS18B20_Init(GPIO_TypeDef * Port,uint16_t Pin);
void DS18B20_ReadId(GPIO_TypeDef * Port,uint16_t Pin,uint8_t *ds18b20_id);
float DS18B20_GetTemp_SkipRom(GPIO_TypeDef * Port,uint16_t Pin);
float DS18B20_GetTemp_MatchRom(GPIO_TypeDef * Port,uint16_t Pin,uint8_t * ds18b20_id);



#else

//#define BSP_DS18B20_PORT             GPIOE
//#define BSP_DS18B20_PIN               GPIO_PIN_2
// 
//#define DS18B20_OUT_1                HAL_GPIO_WritePin(BSP_DS18B20_PORT, BSP_DS18B20_PIN, GPIO_PIN_SET)
//#define DS18B20_OUT_0                HAL_GPIO_WritePin(BSP_DS18B20_PORT, BSP_DS18B20_PIN, GPIO_PIN_RESET)
// 
//#define DS18B20_IN			        HAL_GPIO_ReadPin(BSP_DS18B20_PORT, BSP_DS18B20_PIN)
// 
//uint8_t DS18B20_Init(void);
//void DS18B20_ReadId(uint8_t *ds18b20_id);
//float DS18B20_GetTemp_SkipRom(void);
//float DS18B20_GetTemp_MatchRom(uint8_t * ds18b20_id);


#endif /* DS18B20_PLural  */



#endif	/* __BSP_DS18B20_H */
 

