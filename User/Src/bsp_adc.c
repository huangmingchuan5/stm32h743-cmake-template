#include "bsp_adc.h"
#include <stm32h7xx.h>
#include <stdio.h>

ADC_HandleTypeDef ADC_Handle;
DMA_HandleTypeDef hdma_adc;

/* DMA 的目的缓冲区。
 * 必须放在 DMA 能访问的 SRAM：本工程的 .bss 被链接到 DTCM(0x20000000)，
 * 而 DMA1/DMA2 无法访问 DTCM，DMA 写入无效、CPU 读回恒为 0。
 * 这里用链接脚本新增的 .dma_buffer 段，映射到 D2 SRAM(0x30000000)。
 *
 * 注意：ARMCC(Keil) 的 section(".bss.ARM.__at_0x30000000") 是编译器专有语法，
 *      GCC 不认识，只会生成一个同名自定义段，地址不会被放到 0x30000000。 */
__IO uint16_t ADC_ConvertedValue __attribute__((section(".dma_buffer")));

/* ADC 引脚配置：PC3 模拟输入 */
static void ADC_GPIO_Mode_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* 原来这里只开了 ADC3 的时钟，漏了 GPIOC 的时钟，PC3 根本配置不上 */
	RHEOSTAT_ADC_GPIO_CLK_ENABLE();
	RHEOSTAT_ADC_CLK_ENABLE();

	GPIO_InitStruct.Pin  = RHEOSTAT_ADC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStruct);

	/* PC3 在 BGA 封装内部是 PC3_C，中间有个模拟开关，
	 * 必须先打开才会把引脚连到 ADC3_INP1；LQFP 封装这句无副作用 */
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);
}

/* ADC + DMA 配置 */
static void ADC_Mode_Config(void)
{
	ADC_ChannelConfTypeDef   ADC_Config        = {0};
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit = {0};

	/* ADC 内核时钟取 PLL2P：
	   HSE 25MHz / PLL2M(5) * PLL2N(160) / PLL2P(25) = 32MHz */
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	RCC_PeriphClkInit.PLL2.PLL2M           = 5;
	RCC_PeriphClkInit.PLL2.PLL2N           = 160;
	RCC_PeriphClkInit.PLL2.PLL2P           = 25;
	RCC_PeriphClkInit.PLL2.PLL2Q           = 2;
	RCC_PeriphClkInit.PLL2.PLL2R           = 2;
	RCC_PeriphClkInit.PLL2.PLL2RGE         = RCC_PLL2VCIRANGE_2;
	RCC_PeriphClkInit.PLL2.PLL2VCOSEL      = RCC_PLL2VCOWIDE;
	RCC_PeriphClkInit.PLL2.PLL2FRACN       = 0;
	RCC_PeriphClkInit.AdcClockSelection    = RCC_ADCCLKSOURCE_PLL2;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

	RHEOSTAT_ADC_CLK_ENABLE();
	RHEOSTAT_ADC_DMA_CLK_ENABLE();

	/* DMA：外设到存储器、循环模式、半字对齐、地址都不自增 */
	hdma_adc.Instance                 = RHEOSTAT_ADC_DMA_Base;
	hdma_adc.Init.Request             = RHEOSTAT_ADC_DMA_Request;
	hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_adc.Init.MemInc              = DMA_MINC_DISABLE;
	hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
	hdma_adc.Init.Mode                = DMA_CIRCULAR;
	hdma_adc.Init.Priority            = DMA_PRIORITY_LOW;
	hdma_adc.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

	if (HAL_DMA_Init(&hdma_adc) != HAL_OK)
	{
		printf("<<-ADC-ERROR->> DMA init failed\r\n");
	}

	__HAL_LINKDMA(&ADC_Handle, DMA_Handle, hdma_adc);

	ADC_Handle.Instance                      = RHEOSTAT_ADC;
	ADC_Handle.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV1;   /* 32MHz/1 */
	ADC_Handle.Init.ContinuousConvMode       = ENABLE;                 /* 连续转换 */
	ADC_Handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
	ADC_Handle.Init.DiscontinuousConvMode    = DISABLE;
	ADC_Handle.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
	ADC_Handle.Init.ExternalTrigConv         = ADC_SOFTWARE_START;      /* 软件触发 */
	ADC_Handle.Init.LowPowerAutoWait         = DISABLE;
	ADC_Handle.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
	ADC_Handle.Init.OversamplingMode         = DISABLE;
	ADC_Handle.Init.Resolution               = ADC_RESOLUTION_16B;      /* 16 位，0~65535 */
	ADC_Handle.Init.ScanConvMode             = ADC_SCAN_DISABLE;

	if (HAL_ADC_Init(&ADC_Handle) != HAL_OK)
	{
		printf("<<-ADC-ERROR->> ADC init failed\r\n");
	}

	/* 关键：这几个字段必须显式赋值。
	 * 原来只赋了 4 个字段，OffsetNumber/Offset 是栈上的随机值，
	 * 一旦 OffsetNumber 不等于 ADC_OFFSET_NONE，转换结果会被随机 offset 减掉，
	 * 表现出来就是电压恒为 0。 */
	ADC_Config.Channel                = RHEOSTAT_ADC_CHANNEL;
	ADC_Config.Rank                   = ADC_REGULAR_RANK_1;
	ADC_Config.SamplingTime           = ADC_SAMPLETIME_64CYCLES_5;
	ADC_Config.SingleDiff             = ADC_SINGLE_ENDED;
	ADC_Config.OffsetNumber           = ADC_OFFSET_NONE;
	ADC_Config.Offset                 = 0;
	ADC_Config.OffsetRightShift       = DISABLE;
	ADC_Config.OffsetSignedSaturation = DISABLE;

	if (HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_Config) != HAL_OK)
	{
		printf("<<-ADC-ERROR->> ADC config channel failed\r\n");
	}

	/* H7 必须先做校准（偏移 + 线性度），否则 16 位读数有明显偏差。
	 * 注意：这里原来写的是 HAL_ADCEx_Calibration_GetValue()，
	 * 那个函数只是"读校准因子"，不会执行校准，必须用 Calibration_Start()。 */
	if (HAL_ADCEx_Calibration_Start(&ADC_Handle, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
	{
		printf("<<-ADC-ERROR->> ADC calibration failed\r\n");
	}

	/* 启动 ADC + 循环 DMA：转换结果由硬件直接写进 ADC_ConvertedValue。
	 * 这句内部已经包含 ADC 使能和启动，不需要再调 ADC_Enable() 或 HAL_ADC_Start()。 */
	if (HAL_ADC_Start_DMA(&ADC_Handle, (uint32_t *)&ADC_ConvertedValue, 1) != HAL_OK)
	{
		printf("<<-ADC-ERROR->> ADC start DMA failed\r\n");
	}
}

void ADC_Init(void)
{
	ADC_GPIO_Mode_Config();
	ADC_Mode_Config();
}

/* 原始值换算成毫伏：3.3V 参考、16 位满量程 65535，全程整数运算，
   避免 MicroLIB 下 printf 不支持 %f 的问题 */
uint32_t ADC_GetVoltage_mV(void)
{
	return (uint32_t)ADC_ConvertedValue * 3300U / 65535U;
}
