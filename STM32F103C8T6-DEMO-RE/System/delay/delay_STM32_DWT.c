#include "delay.h"
#include "stm32f10x.h"

// DWT 计数器

int delay_init(void) {
	// 启用 DWT 计数器
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;

	return 0;
}

void delay_us(uint32_t us) {
	if (us == 0) return;

	uint32_t start = DWT->CYCCNT;
	// 计算需要的周期数: (频率 Hz / 1,000,000) * us
    // 为防止溢出，先做除法：freq_in_mhz * us
    const uint32_t ticks = (SystemCoreClock / 1000000) * us;
    
	// 处理可能发生的计数器溢出 (DWT 是 32 位的)
    while ((DWT->CYCCNT - start) < ticks) {
		__NOP();	// 空操作，防止优化
    }
}

void delay_ms(uint32_t ms) {
	if (ms == 0) return;
    uint32_t i;
    for (i = 0; i < ms; i++) {
        delay_us(1000);	// 调用微秒级延时函数
    }
}
