#include "PWM.h"

void PWM_DeInit(void) {
	
}

void PWM_Init(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(PWM_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = PWM_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(PWM_TIMER_CLK, ENABLE);
	
	TIM_InternalClockConfig(PWM_TIMER_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                  //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;              //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;                             //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(PWM_TIMER_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStucture;
	TIM_OCStructInit(&TIM_OCInitStucture);
	TIM_OCInitStucture.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStucture.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStucture.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStucture.TIM_Pulse = 500;    //CCR
	
	TIM_OC1Init(PWM_TIMER_PORT, &TIM_OCInitStucture);
	
	TIM_Cmd(PWM_TIMER_PORT, ENABLE);   //使能定时器
}

void PWM_SetCompare(uint16_t comp) {
	TIM_SetCompare1(PWM_TIMER_PORT, comp);
}
