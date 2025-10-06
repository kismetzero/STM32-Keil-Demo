#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define TIMER_PORT       TIM2
#define TIMER_CLK        RCC_APB1Periph_TIM2

typedef void (*Timer_Callback_t)(void);

void Timer_Init(void);
void Timer_RegisterCallback(Timer_Callback_t cb);

#ifdef __cplusplus
}
#endif

#endif  /* __TIMER_H */