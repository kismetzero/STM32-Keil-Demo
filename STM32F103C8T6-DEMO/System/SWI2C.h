/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWI2C_H
#define __SWI2C_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

void SWI2C_Init(void);
void SWI2C_Start(void);
void SWI2C_Stop(void);
void SWI2C_SendACK(void);
void SWI2C_SendNACK(void);
uint8_t SWI2C_WaitACK(void);
void SWI2C_WriteByte(uint8_t Byte);
uint8_t SWI2C_ReadByte(void);

#ifdef __cplusplus
}
#endif

#endif  /* __SWI2C_H */