#ifndef __SHT40_H
#define __SHT40_H

#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

void SHT40_Init(void);
uint8_t SHT40_Read(void);
float SHT40_GetTemperature(void);
float SHT40_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif  /* __SHT40_H */