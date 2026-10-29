#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"      /* 前提：stm32h7xx_hal_conf.h 里打开了 HAL_ADC_MODULE_ENABLED */

/* 滑动变阻器接在 PA4，对应 ADC1/ADC2 的通道 4 */
#define RHEOSTAT_ADC_PIN					GPIO_PIN_4
#define RHEOSTAT_ADC_GPIO_PORT				GPIOA
#define RHEOSTAT_ADC_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE()

#define RHEOSTAT_ADC_MASTER					ADC1
#define RHEOSTAT_ADC_MASTER_CLK_ENABLE()	__HAL_RCC_ADC12_CLK_ENABLE()
#define RHEOSTAT_ADC_MASTER_CHANNEL			ADC_CHANNEL_4

#define RHEOSTAT_ADC_SLAVE					ADC2
#define RHEOSTAT_ADC_SLAVE_CLK_ENABLE()		__HAL_RCC_ADC12_CLK_ENABLE()
#define RHEOSTAT_ADC_SLAVE_CHANNEL			ADC_CHANNEL_4

#define RHEOSTAT_ADC_DMA_CLK_ENABLE()		__HAL_RCC_DMA1_CLK_ENABLE()
#define RHEOSTAT_ADC_DMA_Base				DMA1_Stream1
#define RHEOSTAT_ADC_DMA_Request			DMA_REQUEST_ADC1

#define RHEOSTAT_ADC_DMA_IRQHandler			DMA1_Stream1_IRQHandler

#define Rheostat_ADC12_IRQ					ADC_IRQn

void Rheostat_ADC_NVIC_Config(void);
void ADC_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */
