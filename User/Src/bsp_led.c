#include <stm32h7xx.h>
#include "bsp_led.h"


void LED_GPIO_Config(void)
{
	
	LEDR_GPIO_CLK_ENABLE();
	LEDG_GPIO_CLK_ENABLE();
	LEDB_GPIO_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin =LEDR_PIN;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LEDR_GPIO_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin =LEDG_PIN;
	HAL_GPIO_Init(LEDG_GPIO_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin =LEDB_PIN;
	HAL_GPIO_Init(LEDB_GPIO_PORT,&GPIO_InitStruct);

	LED_RGBOFF;


		
}

