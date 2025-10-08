#include "Timer.h"

static Timer_Callback_t timer_callback = 0;

void Timer_Init(void) {
	RCC_APB1PeriphClockCmd(TIMER_CLK, ENABLE);  //使能总线时钟
	
	TIM_InternalClockConfig(TIMER_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;            //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;                          //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 10000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMER_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIMER_PORT, TIM_FLAG_Update);        //清除中断标志位，防止初始化后立刻进入中断
	
	TIM_ITConfig(TIMER_PORT, TIM_IT_Update, ENABLE);   //使能中断源
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    //设置NVIC中断分组
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;           //排队优先级（子优先级）
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIMER_PORT, ENABLE);   //使能定时器
}

void Timer_RegisterCallback(Timer_Callback_t cb) {
    timer_callback = cb;
}

void TIM2_IRQHandler() {
	if(TIM_GetITStatus(TIMER_PORT, TIM_IT_Update) == SET) {
		TIM_ClearITPendingBit(TIMER_PORT, TIM_IT_Update);
		if(timer_callback) {
			timer_callback();
		}
	}
}
