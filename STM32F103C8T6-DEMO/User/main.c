#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "PWM.h"

uint16_t i = 0;

int main(void) {
	Delay_Init();
	PWM_Init();
	while(1) {
		for(i = 0; i <= 1000; i++) {
			PWM_SetCompare(i);
			Delay_ms(10);
		}
		Delay_ms(5000);
		for(i = 0; i <= 1000; i++) {
			PWM_SetCompare(1000 - i);
			Delay_ms(10);
		}
		Delay_ms(5000);
	}
}
