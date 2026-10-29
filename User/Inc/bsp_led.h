#ifndef __LED_H
#define __LED_H
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#define LEDR_PIN						GPIO_PIN_0
#define LEDR_GPIO_PORT					GPIOB
#define LEDR_GPIO_CLK_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE()

#define LEDG_PIN						GPIO_PIN_1
#define LEDG_GPIO_PORT					GPIOB
#define LEDG_GPIO_CLK_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE()

#define LEDB_PIN						GPIO_PIN_3
#define LEDB_GPIO_PORT					GPIOA
#define LEDB_GPIO_CLK_ENABLE()			__HAL_RCC_GPIOA_CLK_ENABLE()


#define ON								GPIO_PIN_RESET
#define OFF								GPIO_PIN_SET

#define LEDR(x)							HAL_GPIO_WritePin(LEDR_GPIO_PORT,LEDR_PIN,x)
#define LEDG(x)							HAL_GPIO_WritePin(LEDG_GPIO_PORT,LEDG_PIN,x)
#define LEDB(x)							HAL_GPIO_WritePin(LEDB_GPIO_PORT,LEDB_PIN,x)

#define digitalHi(p,i)					{p->BSRR = i;}					//低16位为置位
#define digitalLo(p,i)					{p->BSRR = (i << 16);}			//高16位为清零
#define digitalToggle(p,i)				{p->ODR	^= i;}					//对该位取反

#define LEDR_TOGGLE						digitalToggle(LEDR_GPIO_PORT,LEDR_PIN)
#define LEDR_OFF						digitalHi(LEDR_GPIO_PORT,LEDR_PIN)
#define LEDR_ON							digitalLo(LEDR_GPIO_PORT,LEDR_PIN)

#define LEDG_TOGGLE						digitalToggle(LEDG_GPIO_PORT,LEDG_PIN)
#define LEDG_OFF						digitalHi(LEDG_GPIO_PORT,LEDG_PIN)
#define LEDG_ON							digitalLo(LEDG_GPIO_PORT,LEDG_PIN)

#define LEDB_TOGGLE						digitalToggle(LEDB_GPIO_PORT,LEDB_PIN)
#define LEDB_OFF						digitalHi(LEDB_GPIO_PORT,LEDB_PIN)
#define LEDB_ON							digitalLo(LEDB_GPIO_PORT,LEDB_PIN)

/* 基本混色，后面高级用法使用PWM可混出全彩颜色,且效果更好 */

//红
#define LED_RED     \
                    do{ LEDR_ON;  LEDG_OFF; LEDB_OFF; }while(0)

//绿
#define LED_GREEN   \
                    do{ LEDR_OFF; LEDG_ON;  LEDB_OFF; }while(0)

//蓝
#define LED_BLUE    \
                    do{ LEDR_OFF; LEDG_OFF; LEDB_ON; }while(0)

//黄(红+绿)
#define LED_YELLOW  \
                    do{ LEDR_ON;  LEDG_ON;  LEDB_OFF; }while(0)

//紫(红+蓝)
#define LED_PURPLE  \
                    do{ LEDR_ON;  LEDG_OFF; LEDB_ON; }while(0)

//青(绿+蓝)
#define LED_CYAN    \
                    do{ LEDR_OFF; LEDG_ON;  LEDB_ON; }while(0)

//白(红+绿+蓝)
#define LED_WHITE   \
                    do{ LEDR_ON;  LEDG_ON;  LEDB_ON; }while(0)

//黑(全部关闭)
#define LED_RGBOFF  \
                    do{ LEDR_OFF; LEDG_OFF; LEDB_OFF; }while(0)

void LED_GPIO_Config(void);

#endif /* __LED_H */





