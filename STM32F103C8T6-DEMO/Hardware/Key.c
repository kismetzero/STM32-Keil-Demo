#include "Key.h"
#include "Delay.h"

void Key_Init(void) {
	RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = KEY_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStructure);
}

uint8_t Key_Scan(void) {
	if(KEY_PRESSED) {
		Delay_ms(10);
		if(KEY_PRESSED) {
			while(KEY_PRESSED);
			return 1;
		}
	}
	return 0;
}
