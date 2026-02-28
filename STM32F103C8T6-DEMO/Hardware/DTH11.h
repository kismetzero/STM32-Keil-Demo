#ifndef __DTH11_H
#define __DTH11_H

#include "stm32f10x.h"

//#define DTH11_GPIO_PORT		GPIOC
//#define DTH11_GPIO_PIN		GPIO_Pin_13
//#define DTH11_GPIO_CLK		RCC_APB2Periph_GPIOC

//#define DTH11_H()				GPIO_SetBits(DTH11_GPIO_PORT, DTH11_GPIO_PIN)
//#define DTH11_L()				GPIO_ResetBits(DTH11_GPIO_PORT, DTH11_GPIO_PIN)
//#define DTH11_READ()			GPIO_ReadInputDataBit(DTH11_GPIO_PORT, DTH11_GPIO_PIN)

#ifdef __cplusplus
	extern "C" {
#endif

//void DTH11_Init(void);
void DHT11_REC_Data(void);
void read_data(uint32_t*data);

#ifdef __cplusplus
}
#endif

#endif  /* __DTH11_H */