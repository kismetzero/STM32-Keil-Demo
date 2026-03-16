#ifndef __SPI_BUS_STM32_STD_LIB_SW_H
#define __SPI_BUS_STM32_STD_LIB_SW_H

#include "spi_bus.h"
#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef struct {
	GPIO_TypeDef*	sck_gpio_port;
	uint16_t		sck_gpio_pin;
	uint32_t		sck_gpio_clk;
	
	GPIO_TypeDef*	mosi_gpio_port;
	uint16_t		mosi_gpio_pin;
	uint32_t		mosi_gpio_clk;
	
	GPIO_TypeDef*	miso_gpio_port;
	uint16_t		miso_gpio_pin;
	uint32_t		miso_gpio_clk;
	
	spi_bus_mode_t mode;
} spi_bus_stm32_std_lib_sw_bus_config_t;

typedef struct {
	GPIO_TypeDef*	cs_gpio_port;
	uint16_t		cs_gpio_pin;
	uint32_t		cs_gpio_clk;
} spi_bus_stm32_std_lib_sw_cs_config_t;

spi_bus_status_t spi_bus_stm32_std_lib_sw_create_bus_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_lib_sw_bus_config_t *cfg);
spi_bus_status_t spi_bus_stm32_std_lib_sw_create_cs_handle(spi_cs_handle_t *handle, spi_bus_stm32_std_lib_sw_cs_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif  /* __SPI_BUS_STM32_STD_LIB_SW_H */