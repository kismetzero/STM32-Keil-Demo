#include "WS2812.h"
#include "stm32f10x.h"

#define LOG_TAG "WS2812"
#include "elog.h"

typedef struct {
	uint32_t		gpio_clk;
	uint16_t		gpio_pin;
	GPIO_TypeDef*	gpio_port;
} ws2812_gpio_config_t;

void WS2812_SendData(ws2812_gpio_config_t *cfg, uint8_t R, uint8_t G, uint8_t B) {
	uint8_t temp[3];
	temp[0] = R; temp[1] = G; temp[2] = B;
	for(uint8_t i = 0; i < 3; i++) {
		for(uint8_t j = 0; j < 8; j++) {
			if (temp[i] & (0x80 >> j)) {
				GPIO_SetBits(cfg->gpio_port, cfg->gpio_pin);
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
				GPIO_ResetBits(cfg->gpio_port, cfg->gpio_pin);
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			} else {
				GPIO_SetBits(cfg->gpio_port, cfg->gpio_pin);
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
				GPIO_ResetBits(cfg->gpio_port, cfg->gpio_pin);
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
				__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			}
		}
	}
}