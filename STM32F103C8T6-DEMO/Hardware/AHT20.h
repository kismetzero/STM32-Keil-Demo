/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AHT20_H
#define __AHT20_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

uint8_t AHT20_Init(void);			// 初始化并校准 return 0 为成功
uint8_t AHT20_Read(void);			// 开启测量并读取 return 0 为成功
uint8_t AHT20_Reset(void);			// 软复位 return 0 为成功
float AHT20_GetTemperature(void);
float AHT20_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */