/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWI2C_H
#define __SWI2C_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define SWI2C_SCL_GPIO_PORT		GPIOB
#define SWI2C_SCL_GPIO_PIN		GPIO_Pin_8
#define SWI2C_SCL_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SWI2C_SDA_GPIO_PORT		GPIOB
#define SWI2C_SDA_GPIO_PIN		GPIO_Pin_9
#define SWI2C_SDA_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SWI2C_SCL_H()			GPIO_SetBits(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN)
#define SWI2C_SCL_L()			GPIO_ResetBits(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN)

#define SWI2C_SDA_H()			GPIO_SetBits(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)
#define SWI2C_SDA_L()			GPIO_ResetBits(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)
#define SWI2C_SDA_READ()		GPIO_ReadInputDataBit(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)

#define SWI2C_SCL_WRITE(x)		GPIO_WriteBit(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN, (BitAction)x)
#define SWI2C_SDA_WRITE(x)		GPIO_WriteBit(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN, (BitAction)x)

void SWI2C_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __SWI2C_H */