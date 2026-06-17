#include "stm32f10x.h"				  // Device header
#include "delay.h"
#include "USART1.h"
#include "AHT20.h"

int main(void) {
	delay_init();
	USART1_Init();
	AHT20_Init();
	
	USART1_printf("Hello World! \n");
	USART1_printf("AHT20-temp: %f", AHT20_GetTemperature());
	while(1) {
		delay_ms(1000);
		AHT20_Read();
		USART1_printf("AHT20-temp: %f", AHT20_GetTemperature());
	}
}
