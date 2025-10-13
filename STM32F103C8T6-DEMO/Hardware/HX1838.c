#include "HX1838.h"

NEC_Data_t NEC_Data;

uint8_t NEC_Decode_Status = 0;
uint8_t NEC_Decode_POS = 0;
uint32_t NEC_Decode_Data = 0;

static inline uint16_t HX1838_GetLowTime(void) {
	
#if (HX1838_TIM_CHANNEL == 1)
	return TIM_GetCapture2(HX1838_TIM_PORT);
#elif (HX1838_TIM_CHANNEL == 2)
	return TIM_GetCapture1(HX1838_TIM_PORT);
#endif
	
}

static inline uint16_t HX1838_GetHighTime(void) {
	
#if (HX1838_TIM_CHANNEL == 1)
	return TIM_GetCapture1(HX1838_TIM_PORT) - TIM_GetCapture2(HX1838_TIM_PORT);
#elif (HX1838_TIM_CHANNEL == 2)
	return TIM_GetCapture2(HX1838_TIM_PORT) - TIM_GetCapture1(HX1838_TIM_PORT);
#endif
	
}

void HX1838_TIM_IRQ_HANDLER(void) {
#if (HX1838_TIM_CHANNEL == 1)
	uint8_t Falling = TIM_GetITStatus(HX1838_TIM_PORT, TIM_IT_CC1) == SET;
	uint8_t Rising = TIM_GetITStatus(HX1838_TIM_PORT, TIM_IT_CC2) == SET;
#elif (HX1838_TIM_CHANNEL == 2)
	uint8_t Falling = TIM_GetITStatus(HX1838_TIM_PORT, TIM_IT_CC2) == SET;
	uint8_t Rising = TIM_GetITStatus(HX1838_TIM_PORT, TIM_IT_CC1) == SET;
#endif
	if(Falling || Rising) {
		uint16_t HT = HX1838_GetHighTime();
		uint16_t LT = HX1838_GetLowTime();
		switch (NEC_Decode_Status) {
			case 0:
				if(Falling) {
					NEC_Decode_Status++;
				}
				break;
			case 1:
				if(Rising && LT > 8000 && LT < 10000) {
					NEC_Decode_Status++;
				} else {
					NEC_Decode_Status = 0;
				}
				break;
			case 2:
				if(Falling) {
					if(HT > 3500 && HT < 5500) {
						NEC_Data.valid = 0;
						NEC_Data.address = 0;
						NEC_Data.command = 0;
						NEC_Data.count = 0;
						NEC_Decode_POS = 0;
						NEC_Decode_Data = 0;
						NEC_Decode_Status++;
					} else if (HT > 1250 && HT < 3250) {
						if(NEC_Data.valid) {
							NEC_Data.count++;
						}
						NEC_Decode_Status = 0;
					} else {
						NEC_Decode_Status = 0;
					}
				} else {
					NEC_Decode_Status = 0;
				}
				break;
			case 3:
				if(Rising) {
					if(LT < 460 || LT > 660) {
						NEC_Decode_Status = 0;
					}	
				} 
				if(Falling) {
					if(HT > 460 && HT < 660) {
						NEC_Decode_Data &= ~(1UL << NEC_Decode_POS);
						NEC_Decode_POS++;
					} else if (HT > 1590 && HT < 1790) {
						NEC_Decode_Data |= 1UL << NEC_Decode_POS;
						NEC_Decode_POS++;
					} else {
						NEC_Decode_Status = 0;
					}
					if(NEC_Decode_POS > 31) {
						NEC_Decode_Status = 0;
						uint8_t addr, addr_inv, cmd, cmd_inv;
						uint8_t *bytes = (uint8_t*)&NEC_Decode_Data;
						addr     = bytes[0];
						addr_inv = bytes[1];
						cmd      = bytes[2];
						cmd_inv  = bytes[3];
						if ((addr ^ addr_inv) == 0xFF && (cmd ^ cmd_inv) == 0xFF) {
							NEC_Data.address = addr;
							NEC_Data.command = cmd;
							NEC_Data.valid = 1;
						} else {
							NEC_Data.address = 0;
							NEC_Data.command = 0;
							NEC_Data.valid = 0;
						}
					}
				}
				break;
		}
	}
	TIM_ClearITPendingBit(HX1838_TIM_PORT, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2);
}

void HX1838_Init(void) {
	//引脚重映射
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(HX1838_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = HX1838_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HX1838_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(HX1838_TIM_CLK, ENABLE);
	
	TIM_InternalClockConfig(HX1838_TIM_PORT);        //使用内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;                  //时钟划分
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;              //计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;                            //周期，ARR计数器
	TIM_TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;   //预分频，PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(HX1838_TIM_PORT, &TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	//TIM_ICStructInit(&TIM_ICInitStructure);
#if (HX1838_TIM_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#elif (HX1838_TIM_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                          //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;     //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  //直连或交叉通道
	//TIM_PWMIConfig(HX1838_TIM_PORT, &TIM_ICInitStructure);
	TIM_ICInit(HX1838_TIM_PORT, &TIM_ICInitStructure);
	
#if (HX1838_TIM_CHANNEL == 1)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
#elif (HX1838_TIM_CHANNEL == 2)
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
#endif
	TIM_ICInitStructure.TIM_ICFilter = 0x1;                            //外部触发数字滤波器
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;        //触发类型
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;              //触发信号分频器
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;  //直连或交叉通道
	TIM_ICInit(HX1838_TIM_PORT, &TIM_ICInitStructure);
	

	TIM_SelectInputTrigger(HX1838_TIM_PORT, TIM_TS_TI2FP2);           //从模式触发源选择
	TIM_SelectSlaveMode(HX1838_TIM_PORT, TIM_SlaveMode_Reset);        //从模式模式选择

	TIM_ITConfig(HX1838_TIM_PORT, TIM_IT_CC1 | TIM_IT_CC2, ENABLE);
	TIM_ClearFlag(HX1838_TIM_PORT, TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    //设置NVIC中断分组
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = HX1838_TIM_IRQ_CHANNEL;  //中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;           //排队优先级（子优先级）
	NVIC_Init(&NVIC_InitStructure);
	
	
	TIM_Cmd(HX1838_TIM_PORT, ENABLE);   //使能定时器
}


