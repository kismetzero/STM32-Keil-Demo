#include "stm32f10x.h"
#include "delay.h"
#include "USART1.h"
#include "spi_bus.h"
#include "spi_bus_stm32_std_lib_sw.h"

int main(void) {
	delay_init();
	USART1_Init();
	USART1_printf("-------aaa--------\n");
	spi_dev_handle_t w25qx;
	spi_bus_handle_t swspi_bus_handle;
	spi_bus_stm32_std_lib_sw_bus_config_t swspi_bus_cfg = {
		.miso_gpio_clk = RCC_APB2Periph_GPIOA,
		.miso_gpio_pin = GPIO_Pin_4,
		.miso_gpio_port = GPIOA,
		.mosi_gpio_clk = RCC_APB2Periph_GPIOA,
		.mosi_gpio_pin = GPIO_Pin_6,
		.mosi_gpio_port = GPIOA,
		.sck_gpio_clk = RCC_APB2Periph_GPIOA,
		.sck_gpio_pin = GPIO_Pin_5,
		.sck_gpio_port = GPIOA
	};
	spi_cs_handle_t swspi_cs_handle;
	spi_bus_stm32_std_lib_sw_cs_config_t swspi_cs_cfg = {
		.cs_gpio_clk = RCC_APB2Periph_GPIOA,
		.cs_gpio_pin = GPIO_Pin_3,
		.cs_gpio_port = GPIOA
	};
	spi_bus_status_t ret;
	ret = spi_bus_stm32_std_lib_sw_create_bus_handle(&swspi_bus_handle, &swspi_bus_cfg);
	USART1_printf("spi_bus_stm32_std_lib_sw_create_bus_handle: %d \n", ret);
	ret = spi_bus_stm32_std_lib_sw_create_cs_handle(&swspi_cs_handle, &swspi_cs_cfg);
	USART1_printf("spi_bus_stm32_std_lib_sw_create_cs_handle: %d \n", ret);
	w25qx.bus = &swspi_bus_handle;
	w25qx.cs = &swspi_cs_handle;
	uint8_t cmd[] = {0x9F};
	uint8_t ret_data[8] = {0};
	
	ret = spi_cs_low(&w25qx);
	USART1_printf("spi_cs_low: %d \n", ret);
	ret = spi_master_transmit(&w25qx, cmd, 1);
	USART1_printf("spi_master_transmit: %d \n", ret);
	
	ret = spi_master_receive(&w25qx, ret_data, 3);
	USART1_printf("spi_master_receive: %d \n", ret);
	ret = spi_cs_high(&w25qx);
	USART1_printf("spi_cs_high: %d \n", ret);
	
	uint16_t did = (ret_data[1] << 8) | ret_data[2];
	USART1_printf("w25q64 mid: %d \n", ret_data[0]);
	USART1_printf("w25q64 did: %d \n", did);
	while(1) {
		delay_ms(1000);
		USART1_printf("-------aaa--------\n");
		
		ret = spi_cs_low(&w25qx);
		USART1_printf("spi_cs_low: %d \n", ret);
		ret = spi_master_transmit(&w25qx, cmd, 1);
		USART1_printf("spi_master_transmit: %d \n", ret);
		
		ret = spi_master_receive(&w25qx, ret_data, 3);
		USART1_printf("spi_master_receive: %d \n", ret);
		ret = spi_cs_high(&w25qx);
		USART1_printf("spi_cs_high: %d \n", ret);
		
		uint16_t did = (ret_data[1] << 8) | ret_data[2];
		USART1_printf("w25q64 mid: %d \n", ret_data[0]);
		USART1_printf("w25q64 did: %d \n", did);
	}
}
