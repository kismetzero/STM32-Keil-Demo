#ifndef __WS2812_H
#define __WS2812_H

#include <stdint.h>
#include "stm32f10x.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    WS2812_STATUS_OK = 0,
	WS2812_STATUS_ERR_GPIO_ERR,
	WS2812_STATUS_ERR_INVALID_PARAM
} WS2812_Status_t;

typedef struct {
	uint16_t led_count;
} WS2812_Handle_t;

void WS2812_Init(WS2812_Handle_t *dev);


#ifdef __cplusplus
}
#endif

#endif  /* __WS2812_H */