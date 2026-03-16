#ifndef __SERIAL_H
#define __SERIAL_H


#ifdef __cplusplus
	extern "C" {
#endif

void Serial_Init(void);
void Serial_Printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  /* __SERIAL_H */