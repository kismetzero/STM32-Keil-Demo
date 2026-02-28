#ifndef __EC11_H
#define __EC11_H

#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

void EC11_Init(void);
uint16_t EC11_GetCount(void);
void EC11_SetCount(uint16_t count);

#ifdef __cplusplus
}
#endif

#endif  /* __EC11_H */