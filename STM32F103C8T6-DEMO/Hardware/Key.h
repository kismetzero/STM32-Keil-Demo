#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define KEY_GPIO_PORT	GPIOA
#define KEY_GPIO_PIN	GPIO_Pin_0
#define KEY_GPIO_CLK	RCC_APB2Periph_GPIOA

#define KEY_RELEASED	(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_GPIO_PIN) == 0)
#define KEY_PRESSED		(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_GPIO_PIN) == 1)

typedef enum {
    KEY_NONE = 0,
    KEY_SINGLE_CLICK,
    KEY_DOUBLE_CLICK,
    KEY_LONG_PRESS
} Key_Event_t;

void Key_Init(void);
uint8_t Key_Scan(void);

#ifdef __cplusplus
}
#endif

#endif  /* __KEY_H */