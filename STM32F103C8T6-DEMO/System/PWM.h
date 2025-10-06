#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define PWM_GPIO_PORT        GPIOA
#define PWM_GPIO_PIN         GPIO_Pin_0
#define PWM_GPIO_CLK         RCC_APB2Periph_GPIOA
#define PWM_TIMER_PORT       TIM2
#define PWM_TIMER_CLK        RCC_APB1Periph_TIM2
#define PWM_TIMER_CHANNEL    TIM_Channel_1

#define PWM_DEFAULT_FREQ    1000    // 1kHz
#define PWM_DEFAULT_DUTY    50.0f   // 50%

void PWM_DeInit(void);
void PWM_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __PWM_H */