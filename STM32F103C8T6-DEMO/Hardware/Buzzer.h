/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define BUZZER_GPIO_PORT	GPIOB
#define BUZZER_GPIO_PIN		GPIO_Pin_1
#define BUZZER_GPIO_CLK		RCC_APB2Periph_GPIOB

//#define BUZZER_ON()			GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN)
//#define BUZZER_OFF()			GPIO_SetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN)
//#define BUZZER_TOGGLE()		(BUZZER_GPIO_PORT->ODR ^= BUZZER_GPIO_PIN)

void Buzzer_Init(void);

static inline void Buzzer_ON(void) {
    GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

static inline void Buzzer_OFF(void) {
    GPIO_SetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

static inline void Buzzer_Toggle(void) {
    if (GPIO_ReadOutputDataBit(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN)) {
        Buzzer_ON();
    } else {
        Buzzer_OFF();
    }
}

#ifdef __cplusplus
}
#endif

#endif  /* __BUZZER_H */