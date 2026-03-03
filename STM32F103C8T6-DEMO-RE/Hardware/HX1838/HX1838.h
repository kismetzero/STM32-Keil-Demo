/**
  ******************************************************************************
  * @file    HX1838.h
  * @author  Kismet
  * @version V0.0.1
  * @date    NOW
  * @brief   HX1838红外接收器，使用TIM的输入捕获、从模式和中断
  ******************************************************************************
  * @attention
  *
  * Copyright (c) ACGBEM.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef __HX1838_H
#define __HX1838_H

#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
	uint8_t valid;
	uint8_t address;
	uint8_t command;
	uint8_t count;
} NEC_Data_t;

extern NEC_Data_t NEC_Data;

void HX1838_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __HX1838_H */