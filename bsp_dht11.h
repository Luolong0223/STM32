#ifndef __DHT11_H___
#define __DHT11_H___
#ifdef __cplusplus
 extern "C" {
#endif

//#include "stm32f4xx_hal.h"

//#include "main.h"
#include "stm32f4xx_hal.h"

#define DHT11_PORT	GPIOC
#define DHT11_PIN		GPIO_PIN_8





#define DHT11_DQ_OUT_LOW		HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_RESET)
#define DHT11_DQ_OUT_HIGH		HAL_GPIO_WritePin(DHT11_PORT,DHT11_PIN,GPIO_PIN_SET)



#define DHT11_DQ_IN	 		HAL_GPIO_ReadPin(DHT11_PORT,DHT11_PIN)






uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint16_t *temp,uint16_t *humi);


#ifdef __cplusplus
}
#endif

#endif


