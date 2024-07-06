
#include "bsp_delay.h"


void bsp_delay_us(uint32_t Delay)
{
  uint32_t cnt = Delay*35; 
  uint32_t i = 0;

	for(i=0;i<cnt;i++)__NOP();
}	;

void SoftDelay_ms(uint16_t a)
{
	HAL_Delay(a);
}

void delay_us(uint32_t Delay)
{
  uint32_t cnt = Delay*35; 
  uint32_t i = 0;

	for(i=0;i<cnt;i++)__NOP();
}	;



