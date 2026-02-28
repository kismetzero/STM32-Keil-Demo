#include "stm32f10x.h"                  // Device header
#include "delay/delay.h"
#include "LED.h"

int main(void) {
	LED_Init();
	delay_init();
	while(1) {
		LED_ON;
		delay_ms(1000);
		LED_OFF;
		delay_ms(1000);
	}
}
