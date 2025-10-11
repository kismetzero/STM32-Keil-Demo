#include "PWM.h"

void PWM_OCDeInit(void) {
	
}

void PWM_OCInit(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(PWM_OC_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = PWM_OC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_OC_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(PWM_OC_TIM_CLK, ENABLE);
	
	TIM_InternalClockConfig(PWM_OC_TIM_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                  //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;              //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;                             //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(PWM_OC_TIM_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStucture;
	TIM_OCStructInit(&TIM_OCInitStucture);
	TIM_OCInitStucture.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStucture.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStucture.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStucture.TIM_Pulse = 500;    //CCR
	
#if (PWM_OC_TIM_CHANNEL == 1)
	TIM_OC1Init(PWM_OC_TIM_PORT, &TIM_OCInitStucture);
#elif (PWM_OC_TIM_CHANNEL == 2)
	TIM_OC2Init(PWM_OC_TIM_PORT, &TIM_OCInitStucture);
#elif (PWM_OC_TIM_CHANNEL == 3)
	TIM_OC3Init(PWM_OC_TIM_PORT, &TIM_OCInitStucture);
#elif (PWM_OC_TIM_CHANNEL == 4)
	TIM_OC4Init(PWM_OC_TIM_PORT, &TIM_OCInitStucture);
#endif
	
	TIM_Cmd(PWM_OC_TIM_PORT, ENABLE);   //使能定时器
}

void PWM_OCSetCompare(uint16_t comp) {
	
#if (PWM_OC_TIM_CHANNEL == 1)
	TIM_SetCompare1(PWM_OC_TIM_PORT, comp);
#elif (PWM_OC_TIM_CHANNEL == 2)
	TIM_SetCompare2(PWM_OC_TIM_PORT, comp);
#elif (PWM_OC_TIM_CHANNEL == 3)
	TIM_SetCompare3(PWM_OC_TIM_PORT, comp);
#elif (PWM_OC_TIM_CHANNEL == 4)
	TIM_SetCompare4(PWM_OC_TIM_PORT, comp);
#endif
	
}

void PWM_OCSetPrescaler(uint16_t presc) {
	TIM_PrescalerConfig(PWM_OC_TIM_PORT, presc, TIM_PSCReloadMode_Immediate);
}

void PWM_OCSetFreq(uint16_t freq) {
	uint16_t presc = (SystemCoreClock / (freq * 1000)) - 1;
	PWM_OCSetPrescaler(presc);
}

void PWM_OCSetDuty(uint16_t duty) {
	PWM_OCSetCompare(duty);
}

void PWM_ICDeInit(void) {
	
}

void PWM_ICInit(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(PWM_IC_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = PWM_IC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_IC_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(PWM_IC_TIM_CLK, ENABLE);
	
	TIM_InternalClockConfig(PWM_IC_TIM_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                  //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;              //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 60000 - 1;                            //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(PWM_IC_TIM_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	//TIM_ICStructInit(&TIM_ICInitStructure);
#if (PWM_IC_TIM_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#elif (PWM_IC_TIM_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#elif (PWM_IC_TIM_CHANNEL == 3)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
#elif (PWM_IC_TIM_CHANNEL == 4)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;      //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  //直连或交叉通道
	//TIM_PWMIConfig(PWM_IC_TIM_PORT, &TIM_ICInitStructure);
	TIM_ICInit(PWM_IC_TIM_PORT, &TIM_ICInitStructure);
	
#if (PWM_IC_TIM_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#elif (PWM_IC_TIM_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#elif (PWM_IC_TIM_CHANNEL == 3)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
#elif (PWM_IC_TIM_CHANNEL == 4)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                            //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;       //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;              //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;  //直连或交叉通道
	TIM_ICInit(PWM_IC_TIM_PORT, &TIM_ICInitStructure);
	
#if (PWM_IC_TIM_CHANNEL == 1)
	TIM_SelectInputTrigger(PWM_IC_TIM_PORT, TIM_TS_TI1FP1);           //从模式触发源选择
	TIM_SelectSlaveMode(PWM_IC_TIM_PORT, TIM_SlaveMode_Reset);        //从模式模式选择
#elif (PWM_IC_TIM_CHANNEL == 2)
	TIM_SelectInputTrigger(PWM_IC_TIM_PORT, TIM_TS_TI2FP2);           //从模式触发源选择
	TIM_SelectSlaveMode(PWM_IC_TIM_PORT, TIM_SlaveMode_Reset);        //从模式模式选择
#endif
	
	TIM_Cmd(PWM_IC_TIM_PORT, ENABLE);   //使能定时器
}

uint32_t PWM_ICGetCapture(void) {
	
#if (PWM_IC_TIM_CHANNEL == 1)
	return TIM_GetCapture1(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 2)
	return TIM_GetCapture2(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 3)
	return TIM_GetCapture3(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 4)
	return TIM_GetCapture4(PWM_IC_TIM_PORT);
#endif
	
}

uint32_t PWM_ICGetCapture2(void) {
	
#if (PWM_IC_TIM_CHANNEL == 1)
	return TIM_GetCapture2(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 2)
	return TIM_GetCapture1(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 3)
	return TIM_GetCapture4(PWM_IC_TIM_PORT);
#elif (PWM_IC_TIM_CHANNEL == 4)
	return TIM_GetCapture3(PWM_IC_TIM_PORT);
#endif
	
}

uint32_t PWM_ICGetFreq(void) {
	return 1000000 / (PWM_ICGetCapture() + 1);
}

uint32_t PWM_ICGetDuty(void) {
	return (PWM_ICGetCapture2() + 1) * 1000 / (PWM_ICGetCapture() + 1);
}
