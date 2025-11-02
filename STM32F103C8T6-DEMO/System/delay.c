#include "Delay.h"

//SysTick中断计时
//volatile uint32_t usTicks = 0;
//void delay_init(void) {
//	// 1000000 us
//	// 1000	ms
//	if (SysTick_Config(SystemCoreClock / 1000000)) {
//		while(1); // 失败死循环
//	}
//}
//void delay_us(uint32_t us)
//{
//	uint32_t start = usTicks;
//	while ((usTicks - start) < us) {
//		__asm("nop"); // 空操作，防止优化
//	}
//}
//void SysTick_Handler(void)
//{
//	usTicks++;
//}

//SysTick不中断计时
void delay_us(uint32_t us) {
	// 计算LOAD寄存器值
	// 使用 AHB 时钟（不分频），更准确
	const uint32_t ticks = (SystemCoreClock / 1000000); // 每微秒多少个时钟
	uint32_t reload = us * ticks;
	  if (reload == 0) return;
	if (reload > 0x00FFFFFF) reload = 0x00FFFFFF; // 限制最大值
	
	// 配置SYSTICK
	SysTick->LOAD  = reload - 1;
	SysTick->VAL   = 0;
	// 时钟源：AHB（不分频），禁止中断，使能
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |  // AHB
					 SysTick_CTRL_ENABLE_Msk;	 // 使能，不开启中断
	// 等待计数到0
	do {
		__asm("nop"); // 空操作，防止优化
	} while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0); // 等待COUNTFLAG置位
	// 关闭SYSTICK
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;
}

//使用TIM定时计数器轮询
//#define DELAY_TIM_PORT		TIM4
//#define DELAY_TIM_CLK		RCC_APB1Periph_TIM4
//void delay_init(void) {
//	RCC_APB1PeriphClockCmd(DELAY_TIM_CLK, ENABLE);	//使能总线时钟
//	
//	TIM_InternalClockConfig(DELAY_TIM_PORT);		//使用内部时钟
//	
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
//	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;					//时钟划分
//	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;				//计数模式
//	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF - 1;							//周期，ARR计数器
//	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;	//预分频，PSC
//	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
//	TIM_TimeBaseInit(DELAY_TIM_PORT, &TIM_TimeBaseInitStructure);
//	
//	TIM_Cmd(DELAY_TIM_PORT, ENABLE);   //使能定时器
//}
//void delay_us(uint32_t us) {
//	uint32_t start = DELAY_TIM_PORT->CNT;
//	while ((DELAY_TIM_PORT->CNT - start) < us) {
//		__asm("nop"); // 空操作，防止优化
//	}
//}

void delay_ms(uint32_t ms) {
	uint32_t i;
	for (i = 0; i < ms; i++) {
		delay_us(1000); // 调用微秒级延时函数
	}
}
