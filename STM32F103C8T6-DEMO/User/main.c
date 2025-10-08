#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "Buzzer.h"

uint16_t i = 0;

int main(void) {
	Delay_Init();
	Buzzer_Init();
	LED_Init();
	while(1) {
		LED_Toggle();
		Delay_ms(500);
		LED_Toggle();
		Delay_ms(500);
	}
}
