#include "bsp_adc.h"
#include <stm32h7xx.h>
#include <stdio.h>

extern float ADC_vol;

ADC_HandleTypeDef ADC_Handle;
ADC_HandleTypeDef ADC_SlAVE_Handle;
DMA_HandleTypeDef hdma_adc;
ADC_MultiModeTypeDef ADC_multimode = {0};
__IO uint32_t ADC_ConvertedValue = 0;


static void ADC_GPIO_Mode_Config(void)
{
	RHEOSTAT_ADC_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStuct;
	GPIO_InitStuct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStuct.Pin = RHEOSTAT_ADC_PIN;
	GPIO_InitStuct.Pull = GPIO_NOPULL;
	
	HAL_GPIO_Init(RHEOSTAT_ADC_GPIO_PORT,&GPIO_InitStuct);
}

static void ADC_Mode_Config(void)
{
	ADC_ChannelConfTypeDef ADC_Config;
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;  
    /*            配置ADC3时钟源             */
    /*    HSE Frequency(Hz)    = 25000000   */                                             
    /*         PLL_M                = 5     */
    /*         PLL_N                = 160   */
    /*         PLL_P                = 25    */
    /*         PLL_Q                = 2     */
    /*         PLL_R                = 2     */
    /*     ADC_ker_clk         = 32000000   */
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    RCC_PeriphClkInit.PLL2.PLL2FRACN = 0;
    RCC_PeriphClkInit.PLL2.PLL2M = 5;
    RCC_PeriphClkInit.PLL2.PLL2N = 160;
    RCC_PeriphClkInit.PLL2.PLL2P = 25;
    RCC_PeriphClkInit.PLL2.PLL2Q = 2;
    RCC_PeriphClkInit.PLL2.PLL2R = 2;
    RCC_PeriphClkInit.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    RCC_PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    RCC_PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2; 
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);  
	
	RHEOSTAT_ADC_MASTER_CLK_ENABLE();
	RHEOSTAT_ADC_SLAVE_CLK_ENABLE();
	RHEOSTAT_ADC_DMA_CLK_ENABLE();
	
	hdma_adc.Instance = RHEOSTAT_ADC_DMA_Base;
	hdma_adc.Init.Request = RHEOSTAT_ADC_DMA_Request;
	hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;				//外设到内存
	hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;	
	hdma_adc.Init.MemInc = DMA_MINC_DISABLE;
	hdma_adc.Init.Mode = DMA_CIRCULAR;
	hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;			//这儿改成了一字
	hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;				//对称的一字，因为没开FIFO模式
	hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_adc.Init.Priority = DMA_PRIORITY_LOW;
	
	HAL_DMA_Init(&hdma_adc);
    //hdma_adc和ADC_Handle.DMA_Handle链接
    __HAL_LINKDMA(&ADC_Handle,DMA_Handle,hdma_adc);     
	
	ADC_Handle.Instance = RHEOSTAT_ADC_MASTER;
	ADC_Handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	ADC_Handle.Init.ContinuousConvMode = ENABLE;
	ADC_Handle.Init.ConversionDataManagement =ADC_CONVERSIONDATA_DMA_CIRCULAR;
	ADC_Handle.Init.DiscontinuousConvMode = DISABLE;
	ADC_Handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	ADC_Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	ADC_Handle.Init.LowPowerAutoWait = DISABLE;
	ADC_Handle.Init.NbrOfConversion = 1;
	ADC_Handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	ADC_Handle.Init.OversamplingMode = DISABLE;
	ADC_Handle.Init.Resolution = ADC_RESOLUTION_16B;
	ADC_Handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
	
	HAL_ADC_Init(&ADC_Handle);
	//ADC校准
    HAL_ADCEx_Calibration_Start(&ADC_Handle,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED); 
	
	
	ADC_SlAVE_Handle.Instance = RHEOSTAT_ADC_SLAVE;
	ADC_SlAVE_Handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	ADC_SlAVE_Handle.Init.ContinuousConvMode = ENABLE;
	ADC_SlAVE_Handle.Init.ConversionDataManagement =ADC_CONVERSIONDATA_DMA_CIRCULAR;
	ADC_SlAVE_Handle.Init.DiscontinuousConvMode = DISABLE;
	ADC_SlAVE_Handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	ADC_SlAVE_Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	ADC_SlAVE_Handle.Init.LowPowerAutoWait = DISABLE;
	ADC_SlAVE_Handle.Init.NbrOfConversion = 1;
	ADC_SlAVE_Handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	ADC_SlAVE_Handle.Init.OversamplingMode = DISABLE;
	ADC_SlAVE_Handle.Init.Resolution = ADC_RESOLUTION_16B;
	ADC_SlAVE_Handle.Init.ScanConvMode = DISABLE;
	HAL_ADC_Init(&ADC_Handle);
	
    HAL_ADCEx_Calibration_Start(&ADC_SlAVE_Handle,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED); 


	//ADC_Config
	ADC_Config.Channel = RHEOSTAT_ADC_MASTER_CHANNEL;
	ADC_Config.Rank = ADC_REGULAR_RANK_1;
	ADC_Config.OffsetNumber = ADC_OFFSET_NONE;
	ADC_Config.SingleDiff = ADC_SINGLE_ENDED;
	ADC_Config.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
	
	HAL_ADC_ConfigChannel(&ADC_Handle,&ADC_Config);
	
	ADC_Config.Channel = RHEOSTAT_ADC_SLAVE_CHANNEL;
	HAL_ADC_ConfigChannel(&ADC_SlAVE_Handle,&ADC_Config);

	//使能ADC1、2
	ADC_Enable(&ADC_Handle);
	ADC_Enable(&ADC_SlAVE_Handle);

	//最后进行配置双重模式
	ADC_multimode.DualModeData = ADC_DUALMODEDATAFORMAT_32_10_BITS;
	ADC_multimode.Mode = ADC_DUALMODE_INTERL;
	ADC_multimode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_3CYCLES;
	
	//DMA双重模式
	HAL_ADCEx_MultiModeConfigChannel(&ADC_Handle,&ADC_multimode);
	
	//使能DMA
	HAL_ADCEx_MultiModeStart_DMA(&ADC_Handle,(uint32_t* )&ADC_ConvertedValue,1);

}

void Rheostat_ADC_NVIC_Config(void)
{
	HAL_NVIC_SetPriority(Rheostat_ADC12_IRQ,1,0);
	HAL_NVIC_EnableIRQ(Rheostat_ADC12_IRQ);
}
	

void ADC_Init(void)
{
	ADC_GPIO_Mode_Config();
	ADC_Mode_Config();
	HAL_ADC_Start(&ADC_Handle);
	HAL_ADC_Start(&ADC_SlAVE_Handle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* Adchandle)
{
	ADC_ConvertedValue = HAL_ADC_GetValue(Adchandle);
}	
