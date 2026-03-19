#include "delay.h"
#include "stm32f10x.h"

#define TIMER_CLK		RCC_APB1Periph_TIM3
#define TIMER_PORT		TIM3

// 使用TIM定时计数器轮询
int delay_init(void) {
	// 使能总线时钟
	// if (TIMER_PORT == TIM1) { RCC_APB2PeriphClockCmd(TIMER_CLK, ENABLE); }
	RCC_APB1PeriphClockCmd(TIMER_CLK, ENABLE);

	// 使用内部时钟
	TIM_InternalClockConfig(TIMER_PORT);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;						// 时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;					// 计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;									// 周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;		// 预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMER_PORT, &TIM_TimeBaseInitStructure);

	// 清除更新标志，防止初始化完成瞬间产生误判
	TIM_ClearFlag(TIMER_PORT, TIM_FLAG_Update);

	// 使能定时器
	TIM_Cmd(TIMER_PORT, ENABLE);

	return 0;
}

void delay_us(uint32_t us) {
	if (us == 0) return;
	uint32_t start = TIMER_PORT->CNT;
	while ((TIMER_PORT->CNT - start) < us) {
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
