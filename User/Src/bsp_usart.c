#include "bsp_usart.h"
#include <stm32h7xx_hal.h>
UART_HandleTypeDef UartHandle;

void UARTx_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;
	
	UARTx_Rx_GPIO_CLK_ENABLE();
	UARTx_Tx_GPIO_CLK_ENABLE();
	
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
	
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);		//配置选择时钟源
	UARTx_CLK_ENABLE();			//打开时钟		
	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pin = UARTx_Rx_PIN;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = UARTx_Rx_AF;
	HAL_GPIO_Init(UARTx_Rx_GPIO_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = UARTx_Tx_PIN;
	GPIO_InitStruct.Alternate = UARTx_Tx_AF;
	HAL_GPIO_Init(UARTx_Tx_GPIO_PORT,&GPIO_InitStruct);
	
	UartHandle.Instance = UARTx;
	UartHandle.Init.BaudRate = UARTx_BAUDRATE;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;	/* 必须显式赋值，否则该字段为0，不在IS_UART_OVERSAMPLING允许的取值内 */
	UartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UartHandle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;	/* 同理，必须显式关闭高级特性 */

	/* 关键：必须调用 HAL_UART_Init()，它才会用上面的 Init 结构真正配置 USART1，并在 CR1 中置位 UE 使能串口 */
	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		while (1);
	}
}

///重定向c库函数printf到串口USARTx，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口USARTx */
	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
	return (ch);
}

///重定向c库函数scanf到串口USARTx，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{	
	int ch;
	/* 等待串口输入数据 */
	while(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) == RESET);
	/* 读 RDR 会自动清 RXNE；此处不要清 ORE(读 RDR 也会清 ORE)，否则可能把刚到的有效数据一起丢掉 */
	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
	return (ch);
}