#include "stm32f10x.h"
#include "delay.h"
#include "i2c_if.h"
#include "AHT20.h"
#include "USART1.h"

extern void i2c_if_SWI2C_CreateHandle(i2c_if_handle_t *handle);

int main(void) {
	delay_init();
	USART1_Init();
	i2c_if_handle_t swi2c;
	i2c_if_SWI2C_CreateHandle(&swi2c);
	swi2c.init(swi2c.user_data);
	AHT20_Init(&swi2c);
	
	USART1_printf("Helllo World!!! \n");
	USART1_printf("AHT20-temp: %f \n", AHT20_GetTemperature());
	USART1_printf("AHT20-hum: %f \n", AHT20_GetHumidity());
	while(1) {
		delay_ms(1000);
		AHT20_Measure();
		USART1_printf("AHT20-temp: %f \n", AHT20_GetTemperature());
		USART1_printf("AHT20-hum: %f \n", AHT20_GetHumidity());
	}
}
