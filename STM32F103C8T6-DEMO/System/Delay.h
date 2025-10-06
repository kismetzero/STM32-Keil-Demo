#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define TIMER_PORT       TIM2
#define TIMER_CLK        RCC_APB1Periph_TIM2

//extern volatile uint32_t usTicks;

void Delay_Init(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif  /* __DELAY_H */