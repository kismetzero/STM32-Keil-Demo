#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define KEY_GPIO_PORT		GPIOA
#define KEY_GPIO_PIN		GPIO_Pin_0
#define KEY_GPIO_CLK		RCC_APB2Periph_GPIOA
#define KEY_PORT_SOURCE		GPIO_PortSourceGPIOA
#define KEY_PIN_SOURC		GPIO_PinSource0
#define KEY_EXTI_LINE		EXTI_Line0
#define KEY_IRQ_HANDLER		EXTI0_IRQHandler
#define KEY_IRQ_CHANNEL		EXTI0_IRQn

// 回调函数类型定义
typedef void (*Key_Callback_t)(void);

void Key_Init(void);
void Key_RegisterCallback(Key_Callback_t cb);

#ifdef __cplusplus
}
#endif

#endif  /* __KEY_H */