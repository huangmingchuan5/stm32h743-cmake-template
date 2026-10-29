#ifndef __ADC_H
#define __ADC_H

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"      /* 前提：stm32h7xx_hal_conf.h 里打开了 HAL_ADC_MODULE_ENABLED */

/* ---------------- 引脚：PC3 对应 ADC3_INP1 ---------------- */
#define RHEOSTAT_ADC_PIN                    GPIO_PIN_3
#define RHEOSTAT_ADC_GPIO_PORT              GPIOC
#define RHEOSTAT_ADC_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

/* ---------------- ADC：使用 ADC3 ---------------- */
#define RHEOSTAT_ADC                        ADC3
#define RHEOSTAT_ADC_CLK_ENABLE()           __HAL_RCC_ADC3_CLK_ENABLE()
#define RHEOSTAT_ADC_CHANNEL                ADC_CHANNEL_1

/* ---------------- DMA：DMA1_Stream1，请求源选 DMAMUX1 的 ADC3 ---------------- */
#define RHEOSTAT_ADC_DMA_CLK_ENABLE()       __HAL_RCC_DMA1_CLK_ENABLE()
#define RHEOSTAT_ADC_DMA_Base               DMA1_Stream1
#define RHEOSTAT_ADC_DMA_Request            DMA_REQUEST_ADC3

/* ADC3 自己的中断向量是 ADC3_IRQn；ADC_IRQn 属于 ADC1/ADC2，不能混用 */
#define RHEOSTAT_ADC_IRQ                    ADC3_IRQn

extern ADC_HandleTypeDef ADC_Handle;
extern DMA_HandleTypeDef hdma_adc;
extern __IO uint16_t     ADC_ConvertedValue;

void     ADC_Init(void);
uint32_t ADC_GetVoltage_mV(void);

#endif
