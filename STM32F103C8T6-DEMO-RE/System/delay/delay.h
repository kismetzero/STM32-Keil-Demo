#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

/**
 * @brief 初始化延时模块 (可选)
 * 
 * @return int 0 表示成功
 * @note 某些平台(如 STM32 使用 DWT 计数器)需要在使用前初始化硬件资源。
 *       如果平台不需要初始化，此函数可为空实现。
 */
int delay_init(void);

/**
 * @brief 微秒级延时
 * 
 * @param us 延时的微秒数 (例如: 10 表示 10us)
 * @note 这是一个阻塞函数。在 RTOS 环境下，极短的延时通常仍通过忙等待实现，
 *       以保证精度；长延时建议由调用者自行判断是否使用任务延时。
 */
void delay_us(uint32_t us);

/**
 * @brief 毫秒级延时
 * 
 * @param ms 延时的毫秒数 (例如: 100 表示 100ms)
 * @note 在 RTOS 环境下，此函数通常会挂起当前任务，让出 CPU 资源。
 *       在裸机环境下，通常通过循环调用 my_delay_us 或定时器实现。
 */
void delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif  /* __DELAY_H */