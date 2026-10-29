#ifndef __RS232_USART_H
#define __RS232_USART_H
#include "stm32h7xx_hal.h"
#include <stdio.h>

#define UARTx							USART1
#define UARTx_CLK_ENABLE()				__HAL_RCC_USART1_CLK_ENABLE()

#define RCC_PERIPHCLK_UARTx				RCC_PERIPHCLK_USART1
#define RCC_UARTxCLKSOURCE_SYSCLK         RCC_USART1CLKSOURCE_SYSCLK

#define UARTx_Rx_GPIO_PORT				GPIOA
#define UARTx_Rx_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE()
#define UARTx_Rx_PIN					GPIO_PIN_9
#define UARTx_Rx_AF						GPIO_AF7_USART1

#define UARTx_Tx_GPIO_PORT				GPIOA
#define UARTx_Tx_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE()
#define UARTx_Tx_PIN					GPIO_PIN_10
#define UARTx_Tx_AF						GPIO_AF7_USART1

#define UARTx_IRQHandler                  USART1_IRQHandler
#define UARTx_IRQ                 		    USART1_IRQn

#define UARTx_BAUDRATE                    115200

extern UART_HandleTypeDef UartHandle;

void UARTx_Config(void);

///重定向c库函数printf到串口USARTx，重定向后可使用printf函数
int fputc(int ch, FILE *f);

///重定向c库函数scanf到串口USARTx，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f);
#endif