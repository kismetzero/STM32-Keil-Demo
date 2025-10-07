#include "LED.h"

void LED_Init(void) {
	RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);
	
	LED_OFF;
}
