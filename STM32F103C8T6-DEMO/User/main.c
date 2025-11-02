#include "stm32f10x.h"				  // Device header
#include "delay.h"
#include "USART1.h"
#include "LED.h"
#include "AHT20.h"

extern uint8_t AHT20_data[6];

int main(void) {
	
	USART1_Init();
	LED_Init();
	LED_ON();
	USART1_printf("Hello World!\n");
	
	delay_ms(1000);
	
	AHT20_Init();
	
	for(uint8_t i = 0; i < 6; i++) {
		USART1_printf("AHT20_data[%d] = %d\n", i, AHT20_data[i]);
	}
	USART1_printf("tmp = %.3f\n", AHT20_GetTemperature());
	USART1_printf("hum = %.3f\n", AHT20_GetHumidity());
	
	AHT20_Read();
	delay_ms(1000);
	USART1_printf("================\n");
	
	for(uint8_t i = 0; i < 6; i++) {
		USART1_printf("AHT20_data[%d] = %d\n", i, AHT20_data[i]);
	}
	USART1_printf("tmp = %.3f\n", AHT20_GetTemperature());
	USART1_printf("hum = %.3f\n", AHT20_GetHumidity());
	
	while(1) {
		
	}
}
