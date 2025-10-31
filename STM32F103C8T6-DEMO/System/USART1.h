/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART1_H
#define __USART1_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define USART1_GPIO_PORT		GPIOA
#define USART1_TX_GPIO_PIN		GPIO_Pin_9
#define USART1_RX_GPIO_PIN		GPIO_Pin_10
#define USART1_GPIO_CLK			RCC_APB2Periph_GPIOA
#define USART1_BaudRate			115200

void USART1_Init(void);
uint8_t USART1_SendByte(uint8_t Byte);

#ifdef __cplusplus
}
#endif

#endif  /* __USART1_H */