/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define DELAY_TIM_PORT		TIM4
#define DELAY_TIM_CLK		RCC_APB1Periph_TIM4

//extern volatile uint32_t usTicks;

void Delay_Init(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif  /* __DELAY_H */