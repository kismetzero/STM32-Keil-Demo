#include "stm32f10x.h"
#include "delay.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std_lib_sw.h"
#include "USART1.h"
#include "AHT20.h"

int main(void) {
	delay_init();
	USART1_Init();
	i2c_bus_handle_t swi2c_handle;
	i2c_bus_stm32_std_lib_sw_config_t swi2c_cfg = {
		.scl_gpio_clk	=	RCC_APB2Periph_GPIOB,
		.scl_gpio_pin	=	GPIO_Pin_8,
		.scl_gpio_port	=	GPIOB,
		.sda_gpio_clk	=	RCC_APB2Periph_GPIOB,
		.sda_gpio_pin	=	GPIO_Pin_9,
		.sda_gpio_port	=	GPIOB
	};
	i2c_bus_stm32_std_lib_sw_create_handle(&swi2c_handle, &swi2c_cfg);
	
	AHT20_Handle_t aht20;
	aht20.hi2c = &swi2c_handle;
	AHT20_Init(&aht20);
	
	USART1_printf("-------bbb--------\n");
	USART1_printf("aht20-temp: %f \n", aht20.temperature);
	USART1_printf("aht20-humi: %f \n", aht20.humidity);
	
	
	while(1) {
		delay_ms(1000);
		AHT20_Measure(&aht20);
		USART1_printf("-------bbb--------\n");
		USART1_printf("aht20-temp: %f \n", aht20.temperature);
		USART1_printf("aht20-humi: %f \n", aht20.humidity);
	}
}
