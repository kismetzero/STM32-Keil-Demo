#include "SG90.h"

#define SG90_GPIO_PORT			GPIOA
#define SG90_GPIO_PIN			GPIO_Pin_0
#define SG90_GPIO_CLK			RCC_APB2Periph_GPIOA
#define SG90_TIM_PORT			TIM2
#define SG90_TIM_CLK			RCC_APB1Periph_TIM2

void SG90_Init(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(SG90_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SG90_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SG90_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(SG90_TIM_CLK, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;					//时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;				//计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 2000 - 1;							//周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler =  (SystemCoreClock / 100000) - 1;	//预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(SG90_TIM_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStucture;
	TIM_OCStructInit(&TIM_OCInitStucture);
	TIM_OCInitStucture.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStucture.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStucture.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStucture.TIM_Pulse = 50;    //CCR
	
	TIM_OC1Init(SG90_TIM_PORT, &TIM_OCInitStucture);
	
	TIM_Cmd(SG90_TIM_PORT, ENABLE);
}

void SG90_SetAngle(uint8_t Angle) {
	uint16_t comp;
	comp = (10 * Angle / 180.0f + 2.5f) / 100.0f * 2000;
	TIM_SetCompare1(SG90_TIM_PORT, comp);
}
