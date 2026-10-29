/**
  ******************************************************************************
  * @file    bsp_key.c
  * @brief   板载按键 BSP 实现
  ******************************************************************************
  */

#include "bsp_key.h"

/**
  * @brief  初始化按键引脚为输入
  * @note   使用 GPIO_NOPULL，依赖开发板上的外部上/下拉电阻
  */
void Key_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    KEY1_GPIO_CLK_ENABLE();
    KEY2_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = KEY1_PIN;
    HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY2_PIN;
    HAL_GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  扫描按键，含 10 ms 软件消抖
  * @retval KEY_ON：本次有效按下；KEY_OFF：未按下
  */
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == KEY_ON)
    {
        /* 消抖：10 ms 后仍为按下状态才认为有效 */
        HAL_Delay(10);

        if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == KEY_ON)
        {
            /* 等待释放，避免一次按下被反复识别 */
            while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == KEY_ON)
            {
                ;
            }
            return KEY_ON;
        }
    }

    return KEY_OFF;
}
