#include "stm32f10x.h"
#include "delay.h"
#include "USART1.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std_lib_sw.h"
#include "SHT40_i2c_bus.h"

void i2c_test(i2c_bus_stm32_std_lib_sw_config_t *cfg);
//void spi_test(spi_bus_stm32_std_lib_sw_bus_config_t *bus_cfg, spi_bus_stm32_std_lib_sw_cs_config_t *cs_cfg);

int main(void) {
	delay_init();
	USART1_Init();
	USART1_printf("-------aaa--------\n");
	i2c_bus_handle_t swi2c_handle;
	i2c_bus_stm32_std_lib_sw_config_t swi2c_cfg;
	i2c_test(&swi2c_cfg);
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_bus_stm32_std_lib_sw_create_handle(&swi2c_handle, &swi2c_cfg);
	USART1_printf("i2c_bus_stm32_std_lib_sw_create_handle: %d \n", i2c_ret);
	
	SHT40_Handle_t sensor;
	SHT40_Status_t ret;
	sensor.hi2c = &swi2c_handle;
	ret = SHT40_Init(&sensor);
	USART1_printf("sensor-init: %d \n", ret);
	USART1_printf("sensor-temp: %f \n", sensor.temperature);
	USART1_printf("sensor-humi: %f \n", sensor.humidity);
	
	while(1) {
		delay_ms(1000);
		USART1_printf("-------aaa--------\n");
		ret = SHT40_Measure(&sensor);
		USART1_printf("sensor-measure: %d \n", ret);
		USART1_printf("sensor-temp: %f \n", sensor.temperature);
		USART1_printf("sensor-humi: %f \n", sensor.humidity);
	}
}

void i2c_test(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	cfg->scl_gpio_clk	=	RCC_APB2Periph_GPIOB;
	cfg->scl_gpio_pin	=	GPIO_Pin_8;
	cfg->scl_gpio_port	=	GPIOB;
	
	cfg->sda_gpio_clk	=	RCC_APB2Periph_GPIOB;
	cfg->sda_gpio_pin	=	GPIO_Pin_9;
	cfg->sda_gpio_port	=	GPIOB;
}

//void spi_test(spi_bus_stm32_std_lib_sw_bus_config_t *bus_cfg, spi_bus_stm32_std_lib_sw_cs_config_t *cs_cfg) {
//	bus_cfg->miso_gpio_clk	=	RCC_APB2Periph_GPIOA;
//	bus_cfg->miso_gpio_pin	=	GPIO_Pin_4;
//	bus_cfg->miso_gpio_port	=	GPIOA;
//	
//	bus_cfg->mosi_gpio_clk	=	RCC_APB2Periph_GPIOA;
//	bus_cfg->mosi_gpio_pin	=	GPIO_Pin_6;
//	bus_cfg->mosi_gpio_port	=	GPIOA;
//	
//	bus_cfg->sck_gpio_clk	=	RCC_APB2Periph_GPIOA;
//	bus_cfg->sck_gpio_pin	=	GPIO_Pin_5;
//	bus_cfg->sck_gpio_port	=	GPIOA;
//	
//	cs_cfg->cs_gpio_clk		=	RCC_APB2Periph_GPIOA;
//	cs_cfg->cs_gpio_pin		=	GPIO_Pin_3;
//	cs_cfg->cs_gpio_port	=	GPIOA;
//}

//void w25qx_test() {
//	spi_dev_handle_t w25qx;
//	spi_bus_handle_t swspi_bus_handle;
//	spi_bus_stm32_std_lib_sw_bus_config_t swspi_bus_cfg;
//	spi_cs_handle_t swspi_cs_handle;
//	spi_bus_stm32_std_lib_sw_cs_config_t swspi_cs_cfg;
//	spi_test(&swspi_bus_cfg, &swspi_cs_cfg);
//	spi_bus_status_t ret;
//	ret = spi_bus_stm32_std_lib_sw_create_bus_handle(&swspi_bus_handle, &swspi_bus_cfg);
//	USART1_printf("spi_bus_stm32_std_lib_sw_create_bus_handle: %d \n", ret);
//	ret = spi_bus_stm32_std_lib_sw_create_cs_handle(&swspi_cs_handle, &swspi_cs_cfg);
//	USART1_printf("spi_bus_stm32_std_lib_sw_create_cs_handle: %d \n", ret);
//	w25qx.bus = &swspi_bus_handle;
//	w25qx.cs = &swspi_cs_handle;
//	uint8_t cmd[] = {0x9F};
//	uint8_t ret_data[8] = {0};
//	
//	ret = spi_cs_low(&w25qx);
//	USART1_printf("spi_cs_low: %d \n", ret);
//	ret = spi_master_transmit(&w25qx, cmd, 1);
//	USART1_printf("spi_master_transmit: %d \n", ret);
//	
//	ret = spi_master_receive(&w25qx, ret_data, 3);
//	USART1_printf("spi_master_receive: %d \n", ret);
//	ret = spi_cs_high(&w25qx);
//	USART1_printf("spi_cs_high: %d \n", ret);
//	
//	uint16_t did = (ret_data[1] << 8) | ret_data[2];
//	USART1_printf("w25q64 mid: %d \n", ret_data[0]);
//	USART1_printf("w25q64 did: %d \n", did);
//}

//void aht20_test() {
//	AHT20_Handle_t sensor;
//	AHT20_Status_t ret;
//	sensor.hi2c = &swi2c_handle;
//	ret = AHT20_Init(&sensor);
//	USART1_printf("sensor-init: %d \n", ret);
//	USART1_printf("sensor-temp: %f \n", sensor.temperature);
//	USART1_printf("sensor-humi: %f \n", sensor.humidity);
//
//	ret = AHT20_Measure(&sensor);
//	USART1_printf("sensor-measure: %d \n", ret);
//	USART1_printf("sensor-temp: %f \n", sensor.temperature);
//	USART1_printf("sensor-humi: %f \n", sensor.humidity);
//}

//void sht40_test() {
//	SHT40_Handle_t sensor;
//	SHT40_Status_t ret;
//	sensor.hi2c = &swi2c_handle;
//	ret = SHT40_Init(&sensor);
//	USART1_printf("sensor-init: %d \n", ret);
//	USART1_printf("sensor-temp: %f \n", sensor.temperature);
//	USART1_printf("sensor-humi: %f \n", sensor.humidity);
//
//	ret = SHT40_Measure(&sensor);
//	USART1_printf("sensor-measure: %d \n", ret);
//	USART1_printf("sensor-temp: %f \n", sensor.temperature);
//	USART1_printf("sensor-humi: %f \n", sensor.humidity);
//}
