#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define PWM_GPIO_PORT		GPIOA
#define PWM_GPIO_PIN		GPIO_Pin_0
#define PWM_GPIO_CLK		RCC_APB2Periph_GPIOA
#define PWM_TIMER_PORT		TIM2
#define PWM_TIMER_CLK		RCC_APB1Periph_TIM2
#define PWM_TIMER_CHANNEL	1						//TIM_Channel_x

void PWM_DeInit(void);
void PWM_Init(void);
void PWM_SetCompare(uint16_t comp);

#ifdef __cplusplus
}
#endif

#endif  /* __PWM_H */