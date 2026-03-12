#ifndef __I2C_BUS_STM32_STD_LIB_SW_H
#define __I2C_BUS_STM32_STD_LIB_SW_H

#include "stm32f10x.h"
#include "i2c_bus.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef struct {
	GPIO_TypeDef*	scl_gpio_port;
	uint16_t		scl_gpio_pin;
	uint32_t		scl_gpio_clk;
	
	GPIO_TypeDef*	sda_gpio_port;
	uint16_t		sda_gpio_pin;
	uint32_t		sda_gpio_clk;
} i2c_bus_stm32_std_lib_sw_config_t;

i2c_bus_status_t i2c_bus_stm32_std_lib_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_lib_sw_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_BUS_STM32_STD_LIB_SW_H */