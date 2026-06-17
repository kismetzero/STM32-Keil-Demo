#include "EC11.h"

#define EC11_GPIO_PORT			GPIOA
#define EC11_S1_GPIO_PIN		GPIO_Pin_6
#define EC11_S2_GPIO_PIN		GPIO_Pin_7
#define EC11_GPIO_CLK			RCC_APB2Periph_GPIOA
#define EC11_TIM_PORT			TIM3
#define EC11_TIM_CLK			RCC_APB1Periph_TIM3

void EC11_Init(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(EC11_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = EC11_S1_GPIO_PIN | EC11_S2_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EC11_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(EC11_TIM_CLK, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;					//时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;				//计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;							//周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;							//预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(EC11_TIM_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	//TIM_ICStructInit(&TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 0xF;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;      //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  //直连或交叉通道
	TIM_ICInit(EC11_TIM_PORT, &TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 0xF;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;      //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  //直连或交叉通道
	TIM_ICInit(EC11_TIM_PORT, &TIM_ICInitStructure);
	
	TIM_EncoderInterfaceConfig(EC11_TIM_PORT, TIM_EncoderMode_TI12, TIM_ICPolarity_Falling, TIM_ICPolarity_Rising);
	
	TIM_Cmd(EC11_TIM_PORT, ENABLE);
}

uint16_t EC11_GetCount(void) {
	return TIM_GetCounter(EC11_TIM_PORT);
}

void EC11_SetCount(uint16_t count) {
	TIM_SetCounter(EC11_TIM_PORT, count);
}
