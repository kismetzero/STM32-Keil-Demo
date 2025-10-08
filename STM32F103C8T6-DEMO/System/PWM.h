#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define PWM_OC_GPIO_PORT		GPIOA
#define PWM_OC_GPIO_PIN			GPIO_Pin_0
#define PWM_OC_GPIO_CLK			RCC_APB2Periph_GPIOA
#define PWM_OC_TIMER_PORT		TIM2
#define PWM_OC_TIMER_CLK		RCC_APB1Periph_TIM2
#define PWM_OC_TIMER_CHANNEL	1						//TIM_Channel_x

#define PWM_IC_GPIO_PORT		GPIOA
#define PWM_IC_GPIO_PIN			GPIO_Pin_6
#define PWM_IC_GPIO_CLK			RCC_APB2Periph_GPIOA
#define PWM_IC_TIMER_PORT		TIM3
#define PWM_IC_TIMER_CLK		RCC_APB1Periph_TIM3
#define PWM_IC_TIMER_CHANNEL	1						//TIM_Channel_x

void PWM_OCDeInit(void);
void PWM_OCInit(void);
void PWM_OCSetCompare(uint16_t comp);

void PWM_ICInit(void);
uint32_t TIC_GetCapture();
uint32_t TIC_GetCapture2();

#ifdef __cplusplus
}
#endif

#endif  /* __PWM_H */