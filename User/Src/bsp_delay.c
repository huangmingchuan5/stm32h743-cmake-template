/**
  ******************************************************************************
  * @file    bsp_delay.c
  * @brief   基于 DWT 的微秒延时实现
  ******************************************************************************
  * 原理：Cortex-M7 的 DWT（Data Watchpoint and Trace）单元带一个 32 位
  *       CYCCNT 计数器，每个内核时钟周期加 1。延时就变成「数够 N 个周期」。
  *
  *       周期数 = 微秒数 × (SystemCoreClock / 1 000 000)
  *       480 MHz 下即每微秒 480 个周期。
  *
  * 注意：CYCCNT 是 32 位，480 MHz 下约 8.9 秒回绕一次。本实现按 1000 us
  *       分段，既能覆盖较长延时，也避免回绕导致的计时错误。
  ******************************************************************************
  */

#include "bsp_delay.h"

void BSP_DelayInit(void)
{
    /* 使能 DWT 与 ITM 的跟踪功能 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 计数器清零并使能 */
    DWT->CYCCNT = 0U;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

void BSP_DelayUs(uint32_t us)
{
    const uint32_t cycles_per_us = SystemCoreClock / 1000000U;

    if (us == 0U || cycles_per_us == 0U)
    {
        return;
    }

    while (us > 0U)
    {
        /* 分段，避免 32 位计数器回绕 */
        uint32_t chunk = (us > 1000U) ? 1000U : us;
        uint32_t ticks = chunk * cycles_per_us;
        uint32_t start = DWT->CYCCNT;

        while ((DWT->CYCCNT - start) < ticks)
        {
            ;
        }

        us -= chunk;
    }
}
