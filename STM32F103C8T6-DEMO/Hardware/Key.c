#include "Key.h"
#include "Delay.h"

typedef enum {
    KEY_NONE = 0,
    KEY_SINGLE_CLICK,
    KEY_DOUBLE_CLICK,
    KEY_LONG_PRESS
} Key_Event_t;

// 按键状态枚举
typedef enum {
    KEY_IDLE,      // 未按下
    KEY_DEBOUNCE,  // 消抖中
    KEY_PRESSED    // 已确认按下（等待释放）
} Key_State_t;

static Key_Callback_t key_callback = 0;

static Key_State_t key_state = KEY_IDLE;

void Key_Init(void) {
	RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = KEY_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
	
}

