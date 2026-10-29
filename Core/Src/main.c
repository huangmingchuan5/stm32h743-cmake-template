#include "stm32h7xx.h"
#include "main.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "core_delay.h"
#include "bsp_mpu.h"
#include "bsp_usart.h"
#include "bsp_adc.h"


extern __IO uint32_t ADC_ConvertedValue;
__IO uint16_t ADC_ConvertedValueLocal[2];
float ADC_vol[2];

/* 函数定义在本文件末尾，需前置声明 */
void SystemClock_Config(void);

/**
  * @brief  主函数
  */
int main(void)
{
	/* HAL_Init() 必须最先调用：
	   它负责 FLASH 预取 / 中断优先级分组，并调用 HAL_InitTick()。
	   本工程的 core_delay.c 把 HAL_InitTick 重定向到 DWT 初始化，
	   漏掉这一步 DWT 不会被使能，HAL_GetTick() 恒为 0，HAL_Delay() 会永久卡死。 */
	HAL_Init();

	SystemClock_Config();

	UARTx_Config();
	LED_GPIO_Config();
	Key_GPIO_Config();
	ADC_Init();

	printf("\r\nSTM32H743 CMake template started.\r\n");
	printf("ADC dual-mode sampling on PA4 (ADC1/ADC2 channel 4)\r\n");
	
	while(1)
	{
		HAL_Delay(100);
    LEDB_ON;
    HAL_Delay(3000);
    LEDB_OFF;
    HAL_UART_Transmit(&UartHandle, (uint8_t*)"TEST\r\n", 7, 100);
		ADC_ConvertedValueLocal[0] = (uint16_t)ADC_ConvertedValue;
		ADC_ConvertedValueLocal[1] = (uint16_t)((ADC_ConvertedValue&0xFFFF0000)>>16);
		
		ADC_vol[0] =(float)((uint16_t)ADC_ConvertedValueLocal[0]*3.3/65536); 
		ADC_vol[1] =(float)((uint16_t)ADC_ConvertedValueLocal[1]*3.3/65536);  
    LEDG_ON;
    HAL_Delay(1000);
    LEDG_OFF;
    printf("\r\n The current AD value = 0x%08X \r\n", ADC_ConvertedValueLocal[0]); 
    printf("\r\n The current AD value = 0x%08X \r\n", ADC_ConvertedValueLocal[1]);     
    //读取转换的AD值
    printf("\r\n The current ADC1 value = %f V \r\n",ADC_vol[0]); 
    printf("\r\n The current ADC2 value = %f V \r\n",ADC_vol[1]); 
    LED_WHITE;
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
