#include "delay.h"
#include "stm32f10x.h"

// SysTick 不中断计时
int delay_init(void) {
	// 时钟源：AHB（不分频），禁止中断，使能
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |	// AHB
					SysTick_CTRL_ENABLE_Msk;		// 使能，不开启中断
	return 0;
}

void delay_us(uint32_t us) {
	if (us == 0) return;
	// 计算 LOAD 寄存器值
	// 使用 AHB 时钟（不分频），更准确
	uint32_t reload = (SystemCoreClock / 1000000) * us;
	if (reload > 0x00FFFFFF) reload = 0x00FFFFFF; // 限制最大值
	// 配置 SYSTICK
	SysTick->LOAD  = reload - 1;
	SysTick->VAL   = 0;
	// 等待 COUNTFLAG 置位 (表示计数到0)
	// 注意：读取 CTRL 寄存器会自动清除 COUNTFLAG，所以循环条件里读即可
	while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
		__NOP();	// 空操作，防止优化
	};
}

void delay_ms(uint32_t ms) {
	if (ms == 0) return;
	uint32_t i;
	for (i = 0; i < ms; i++) {
		delay_us(1000);	// 调用微秒级延时函数
	}
}
