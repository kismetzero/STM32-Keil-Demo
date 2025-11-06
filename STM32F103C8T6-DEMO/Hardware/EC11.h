/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EC11_H
#define __EC11_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

void EC11_Init(void);
uint16_t EC11_GetCount(void);
void EC11_SetCount(uint16_t count);

#ifdef __cplusplus
}
#endif

#endif  /* __EC11_H */