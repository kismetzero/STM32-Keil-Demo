#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
	extern "C" {
#endif

// 回调函数类型定义
typedef void (*Timer_Callback_t)(void);

void Timer_Init(void);
void Timer_ITInit(void);
void Timer_ITRegisterCallback(Timer_Callback_t cb);

#ifdef __cplusplus
}
#endif

#endif	/* __TIMER_H */