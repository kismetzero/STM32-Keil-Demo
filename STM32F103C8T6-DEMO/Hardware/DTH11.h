#ifndef __DTH11_H
#define __DTH11_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define DTH11_GPIO_PORT    GPIOC
#define DTH11_GPIO_PIN     GPIO_Pin_13
#define DTH11_GPIO_CLK     RCC_APB2Periph_GPIOC

#define DTH11_HIGH    GPIO_SetBits(DTH11_GPIO_PORT, DTH11_GPIO_PIN)
#define DTH11_LOW     GPIO_ResetBits(DTH11_GPIO_PORT, DTH11_GPIO_PIN)
#define DTH11_DATA    GPIO_ReadInputDataBit(DTH11_GPIO_PORT, DTH11_GPIO_PIN)

void DTH11_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __DTH11_H */