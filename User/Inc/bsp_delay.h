/**
  ******************************************************************************
  * @file    bsp_delay.h
  * @brief   基于 DWT 周期计数器的微秒级延时
  ******************************************************************************
  * 与野火 core_delay 的区别：本实现不重定义 HAL_GetTick / HAL_InitTick，
  * 因此不会破坏 HAL_Delay 与 HAL 的时基，可与现有工程共存。
  ******************************************************************************
  */

#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
  * @brief  使能 DWT 周期计数器
  * @note   只需在 main() 初始化阶段调用一次
  */
void BSP_DelayInit(void);

/**
  * @brief  微秒级延时（忙等，不进入低功耗）
  * @param  us 延时长度，单位微秒
  * @note   依赖 BSP_DelayInit() 已调用；基于 SystemCoreClock 计算周期数
  */
void BSP_DelayUs(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DELAY_H */
