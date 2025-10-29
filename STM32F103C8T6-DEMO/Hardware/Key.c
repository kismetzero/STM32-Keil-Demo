#include "Key.h"

typedef enum {
	KEY_NONE = 0,
	KEY_SINGLE_CLICK,
	KEY_DOUBLE_CLICK,
	KEY_LONG_PRESS
} Key_Event_t;

// 按键状态枚举
typedef enum {
	KEY_IDLE,	 	// 未按下
	KEY_DEBOUNCE,	// 消抖中
	KEY_PRESSED		// 已确认按下（等待释放）
} Key_State_t;

static Key_Callback_t key_callback = 0;

static Key_State_t key_state = KEY_IDLE;

void KEY_IRQ_HANDLER(void) {
	if(EXTI_GetITStatus(KEY_EXTI_LINE) == SET) {
		EXTI_ClearITPendingBit(KEY_EXTI_LINE);
		if(key_callback) {
			key_callback();
		}
	}
}

void Key_Init(void) {
	RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = KEY_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(KEY_PORT_SOURCE, KEY_PIN_SOURC);
	
	EXTI_InitTypeDef EXTI_InitStructure;
	//EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = KEY_IRQ_CHANNEL;		//中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		   //排队优先级（子优先级）
	NVIC_Init(&NVIC_InitStructure);
}

void Key_RegisterCallback(Key_Callback_t cb) {
	key_callback = cb;
}

