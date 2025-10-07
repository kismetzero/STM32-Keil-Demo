#ifndef __TIC_H
#define __TIC_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define TIC_GPIO_PORT		GPIOA
#define TIC_GPIO_PIN		GPIO_Pin_6
#define TIC_GPIO_CLK		RCC_APB2Periph_GPIOA
#define TIC_TIMER_PORT		TIM3
#define TIC_TIMER_CLK		RCC_APB1Periph_TIM3
#define TIC_TIMER_CHANNEL	1						//TIM_Channel_x

void TIC_Init(void);
uint32_t TIC_GetCapture();
uint32_t TIC_GetCapture2();

#ifdef __cplusplus
}
#endif

#endif  /* __TICL_H */