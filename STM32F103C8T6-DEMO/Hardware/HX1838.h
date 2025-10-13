#ifndef __HX1838_H
#define __HX1838_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define HX1838_GPIO_PORT		GPIOA
#define HX1838_GPIO_PIN			GPIO_Pin_1
#define HX1838_GPIO_CLK			RCC_APB2Periph_GPIOA
#define HX1838_TIM_PORT			TIM2
#define HX1838_TIM_CLK			RCC_APB1Periph_TIM2
#define HX1838_TIM_CHANNEL		2						//TIM_Channel_x
#define HX1838_TIM_IRQ_HANDLER	TIM2_IRQHandler
#define HX1838_TIM_IRQ_CHANNEL	TIM2_IRQn

typedef struct {
	uint8_t valid;
	uint8_t address;
	uint8_t command;
	uint8_t count;
} NEC_Data_t;

extern NEC_Data_t NEC_Data;

void HX1838_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __HX1838_H */