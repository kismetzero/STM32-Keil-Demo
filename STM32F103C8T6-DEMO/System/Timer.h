#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define TIMER_TIM_PORT				TIM4
#define TIMER_TIM_CLK				RCC_APB1Periph_TIM4
#define TIMER_TIM_IRQ_HANDLER		TIM4_IRQHandler
#define TIMER_TIM_IRQ_CHANNEL		TIM4_IRQn
#define TIMER_TIM_IRQ_FLAG			TIM_IT_Update

// 回调函数类型定义
typedef void (*Timer_Callback_t)(void);

void Timer_Init(void);
void Timer_ITInit(void);
void Timer_ITRegisterCallback(Timer_Callback_t cb);
#ifdef __cplusplus
}
#endif

#endif  /* __TIMER_H */