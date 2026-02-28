#ifndef __SG90_H
#define __SG90_H

#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

void SG90_Init(void);
void SG90_SetAngle(uint8_t Angle);

#ifdef __cplusplus
}
#endif

#endif  /* __SG90_H */