#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 回调函数类型定义
typedef void (*Key_Callback_t)(void);

void Key_Init(void);
void Key_RegisterCallback(Key_Callback_t cb);

#ifdef __cplusplus
}
#endif

#endif  /* __KEY_H */