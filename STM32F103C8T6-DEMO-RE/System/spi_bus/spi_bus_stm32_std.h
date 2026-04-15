#ifndef __SPI_BUS_STM32_STD_H
#define __SPI_BUS_STM32_STD_H

#include "spi_bus.h"
#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
	GPIO_TypeDef*	sck_gpio_port;
	GPIO_TypeDef*	mosi_gpio_port;
	GPIO_TypeDef*	miso_gpio_port;
	uint32_t		sck_gpio_clk;
	uint32_t		mosi_gpio_clk;
	uint32_t		miso_gpio_clk;
	spi_bus_mode_t	mode;
	uint16_t		sck_gpio_pin;
	uint16_t		mosi_gpio_pin;
	uint16_t		miso_gpio_pin;
} spi_bus_stm32_std_sw_config_t;

spi_bus_status_t spi_bus_stm32_std_sw_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_sw_config_t *cfg);

typedef struct {
	uint32_t		cs_gpio_clk;
	uint16_t		cs_gpio_pin;
	GPIO_TypeDef*	cs_gpio_port;
} spi_bus_stm32_std_cs_config_t;

spi_bus_status_t spi_bus_stm32_std_cs_create_handle(spi_cs_handle_t *handle, spi_bus_stm32_std_cs_config_t *cfg);

typedef struct {
	SPI_TypeDef*	spi_periph;
	GPIO_TypeDef*	spi_gpio_port;
	uint32_t		spi_clk;
	spi_bus_mode_t mode;
	uint32_t		spi_gpio_clk;
	uint16_t		sck_gpio_pin;
	uint16_t		mosi_gpio_pin;
	uint16_t		miso_gpio_pin;
} spi_bus_stm32_std_hw_config_t;

spi_bus_status_t spi_bus_stm32_std_hw_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_hw_config_t *cfg);


typedef struct {
	spi_bus_mode_t mode;
} spi_bus_stm32_std_SW_config_t;

spi_bus_status_t spi_bus_stm32_std_SW_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_SW_config_t *cfg);


typedef struct {
	spi_bus_mode_t mode;
} spi_bus_stm32_std_HW_config_t;

spi_bus_status_t spi_bus_stm32_std_HW_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_HW_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif	/* __SPI_BUS_STM32_STD_H */