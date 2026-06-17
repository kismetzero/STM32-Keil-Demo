#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

void Serial_Init(uint32_t BaudRate);
void Serial_SendByte(uint8_t byte);
void Serial_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	/* __SERIAL_H */