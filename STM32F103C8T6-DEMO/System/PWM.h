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
#define PWM_OC_TIM_PORT			TIM2
#define PWM_OC_TIM_CLK			RCC_APB1Periph_TIM2
#define PWM_OC_TIM_CHANNEL		1						//TIM_Channel_x

#define PWM_IC_GPIO_PORT		GPIOA
#define PWM_IC_GPIO_PIN			GPIO_Pin_6
#define PWM_IC_GPIO_CLK			RCC_APB2Periph_GPIOA
#define PWM_IC_TIM_PORT			TIM3
#define PWM_IC_TIM_CLK			RCC_APB1Periph_TIM3
#define PWM_IC_TIM_CHANNEL		1						//TIM_Channel_x

void PWM_OCDeInit(void);
void PWM_OCInit(void);
void PWM_OCSetCompare(uint16_t comp);
void PWM_OCSetPrescaler(uint16_t presc);
void PWM_OCSetFreq(uint16_t freq);
void PWM_OCSetDuty(uint16_t duty);

void PWM_ICInit(void);
uint32_t PWM_ICGetFreq();
uint32_t PWM_ICGetDuty();

#ifdef __cplusplus
}
#endif

#endif  /* __PWM_H */