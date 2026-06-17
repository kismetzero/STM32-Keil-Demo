#ifndef __USART1_H
#define __USART1_H

#include "stdint.h"

#ifdef __cplusplus
	extern "C" {
#endif

void USART1_Init(void);
uint8_t USART1_SendByte(uint8_t Byte);
void USART1_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  /* __USART1_H */