#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

//#define DHT11_GPIO_PORT		GPIOC
//#define DHT11_GPIO_PIN		GPIO_Pin_13
//#define DHT11_GPIO_CLK		RCC_APB2Periph_GPIOC

//#define DHT11_H()				GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)
//#define DHT11_L()				GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)
//#define DHT11_READ()			GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN)

#ifdef __cplusplus
	extern "C" {
#endif

//void DHT11_Init(void);
void DHT11_REC_Data(void);
void read_data(uint32_t*data);

#ifdef __cplusplus
}
#endif

#endif  /* __DHT11_H */