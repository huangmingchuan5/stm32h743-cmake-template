/**
  ******************************************************************************
  * @file    bsp_key.h
  * @brief   板载按键 BSP（H7Pro：KEY1=PA0，KEY2=PC13）
  ******************************************************************************
  * 接口与野火教程保持一致，便于对照学习；实现为独立编写。
  *
  * 极性说明：本例按「按下为高电平」处理（KEY_ON=1），与野火 H7Pro 例程一致。
  *           若你的板子按键按下接地（低电平），把 KEY_ON 改为 0、KEY_OFF 改为 1。
  ******************************************************************************
  */

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32h7xx_hal.h"
/* KEY1：PA0（WK_UP） */
#define KEY1_PIN                    GPIO_PIN_0
#define KEY1_GPIO_PORT              GPIOA
#define KEY1_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

/* KEY2：PC13 */
#define KEY2_PIN                    GPIO_PIN_13
#define KEY2_GPIO_PORT              GPIOC
#define KEY2_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

/* 按下 / 释放的电平判定值 */
#define KEY_ON                      1
#define KEY_OFF                     0

void Key_GPIO_Config(void);

/**
  * @brief  扫描指定按键
  * @param  GPIOx 端口，如 KEY1_GPIO_PORT
  * @param  GPIO_Pin 引脚，如 KEY1_PIN
  * @retval KEY_ON 表示本次按下（已消抖，且等待到释放），KEY_OFF 表示未按下
  * @note   函数内部会阻塞等待按键释放
  */
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_KEY_H */
