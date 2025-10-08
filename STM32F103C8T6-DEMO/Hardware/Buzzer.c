#include "Buzzer.h"

void Buzzer_Init(void) {
	RCC_APB2PeriphClockCmd(BUZZER_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = BUZZER_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStructure);
	
	Buzzer_OFF();
}
