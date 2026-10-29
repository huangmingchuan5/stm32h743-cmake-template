#include "stm32h7xx.h"
#include "main.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "core_delay.h"
#include "bsp_mpu.h"
#include "bsp_usart.h"
#include "bsp_adc.h"


/* ADC_ConvertedValue 已在 bsp_adc.h 中声明为 __IO uint16_t，此处不再重复声明，
   否则会出现 conflicting types（uint32_t vs uint16_t） */
__IO uint16_t ADC_ConvertedValueLocal[2];
float ADC_vol[2];

/* 函数定义在本文件末尾，需前置声明 */
void SystemClock_Config(void);

/**
  * @brief  主函数
  */
int main(void)
{
	uint32_t raw;
	uint32_t mv;

	/* 系统时钟初始化成 480MHz */
	SystemClock_Config();

	/* LED 端口初始化 */
	LED_GPIO_Config();

	/* 串口1：115200 8-N-1 */
	UARTx_Config();

	/* ADC3 + DMA 初始化，初始化完就已经在后台连续采样 */
	ADC_Init();

	printf("\r\n==== STM32H743 ADC3 单通道(DMA)采集电压 ====\r\n");
	printf("通道：PC3 -> ADC3_INP1，分辨率 16 位，参考电压 3.3V\r\n");
	printf("转动板上电位器，下面的电压值应随之变化\r\n\r\n");

	while (1)
	{
		/* DMA 一直在后台更新这个变量，直接读即可 */
		raw = ADC_ConvertedValue;
		mv  = (uint32_t)raw * 3300U / 65535U;

		printf("ADC = %5u (0x%04X)  Voltage = %u.%03u V\r\n",
		       (unsigned)raw, (unsigned)raw,
		       (unsigned)(mv / 1000U), (unsigned)(mv % 1000U));

		/* 顺手用彩灯做个粗略指示：电压高于中点亮绿灯，否则亮红灯 */
		if (raw > 32767U)
		{
			LED_GREEN;
		}
		else
		{
			LED_RED;
		}

		HAL_Delay(500);
	}
}

/**
  * @brief  System Clock 配置（与工程模板完全一致，未改动）
  *         SYSCLK = 480MHz，HCLK = 240MHz，APB1/2/3/4 = 120MHz
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* 启用电源配置更新 */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /* 配置主内稳压器输出电压 */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 5;
  RCC_OscInitStruct.PLL.PLLN       = 192;
  RCC_OscInitStruct.PLL.PLLP       = 2;
  RCC_OscInitStruct.PLL.PLLQ       = 2;
  RCC_OscInitStruct.PLL.PLLR       = 2;
  RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN   = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    while(1);
  }
}
