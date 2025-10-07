#include "TIC.h"

void TIC_Init(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(TIC_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = TIC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TIC_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(TIC_TIMER_CLK, ENABLE);
	
	TIM_InternalClockConfig(TIC_TIMER_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                  //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;              //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 60000 - 1;                            //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIC_TIMER_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
#if (TIC_TIMER_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#elif (TIC_TIMER_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#elif (TIC_TIMER_CHANNEL == 3)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
#elif (TIC_TIMER_CHANNEL == 4)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;      //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  //直连或交叉通道
	//TIM_PWMIConfig(TIC_TIMER_PORT, &TIM_ICInitStructure);
	TIM_ICInit(TIC_TIMER_PORT, &TIM_ICInitStructure);
	
#if (TIC_TIMER_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#elif (TIC_TIMER_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#elif (TIC_TIMER_CHANNEL == 3)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
#elif (TIC_TIMER_CHANNEL == 4)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;      //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;  //直连或交叉通道
	TIM_ICInit(TIC_TIMER_PORT, &TIM_ICInitStructure);
	
	TIM_SelectInputTrigger(TIC_TIMER_PORT, TIM_TS_TI1FP1);           //从模式触发源选择
	TIM_SelectSlaveMode(TIC_TIMER_PORT, TIM_SlaveMode_Reset);        //从模式模式选择
	
	TIM_Cmd(TIC_TIMER_PORT, ENABLE);   //使能定时器
}

uint32_t TIC_GetCapture() {
	
#if (TIC_TIMER_CHANNEL == 1)
	return TIM_GetCapture1(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 2)
	return TIM_GetCapture2(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 3)
	return TIM_GetCapture3(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 4)
	return TIM_GetCapture4(TIC_TIMER_PORT);
#endif
	
}

uint32_t TIC_GetCapture2() {
	
#if (TIC_TIMER_CHANNEL == 1)
	return TIM_GetCapture2(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 2)
	return TIM_GetCapture1(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 3)
	return TIM_GetCapture4(TIC_TIMER_PORT);
#elif (TIC_TIMER_CHANNEL == 4)
	return TIM_GetCapture3(TIC_TIMER_PORT);
#endif
	
}
