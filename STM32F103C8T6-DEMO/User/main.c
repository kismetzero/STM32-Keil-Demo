#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "PWM.h"
#include "OLED.h"

int main(void) {
	Delay_Init();
	LED_Init();
	/*OLED初始化*/
	OLED_Init();
	OLED_ShowString(0, 0, "Freq: 00000Hz", OLED_8X16);
	OLED_ShowString(0, 18, "Duty: 0000%", OLED_8X16);
	OLED_Update();
	while(1) {
	}
}
