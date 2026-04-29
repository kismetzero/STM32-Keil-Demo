#ifndef __I2C_BUS_STM32_STD_H
#define __I2C_BUS_STM32_STD_H

#include "i2c_bus.h"
#include "stm32f10x.h"

#define SYS_USE_FREERTOS
#include "sys_inc.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
	#if SYS_EN_FREERTOS
		SemaphoreHandle_t mutex_lock;
	#endif /* SYS_EN_FREERTOS */
	GPIO_TypeDef*	scl_gpio_port;
	GPIO_TypeDef*	sda_gpio_port;
	uint32_t		scl_gpio_clk;
	uint32_t		sda_gpio_clk;
	uint16_t		scl_gpio_pin;
	uint16_t		sda_gpio_pin;
	uint8_t			inited;
} i2c_bus_stm32_std_sw_config_t;

i2c_bus_status_t i2c_bus_stm32_std_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_sw_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_BUS_STM32_STD_H */