#ifndef __MAX7219_H
#define __MAX7219_H

#include <stdint.h>

#define SYS_USE_FREERTOS
#include "sys_inc.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    MAX7219_STATUS_OK = 0,
	MAX7219_STATUS_ERR,
	MAX7219_STATUS_ERR_INVALID_PARAM,
	MAX7219_STATUS_ERR_NO_INIT,
	MAX7219_STATUS_ERR_SPI_ERR,
} MAX7219_Status_t;

#define MAX7219_REG_EN				0x0CU	// shutdown		关断寄存器
#define MAX7219_REG_DECODE_MODE		0x09U	// decode mode	译码模式寄存器
#define MAX7219_REG_SCAN_LIMIT		0x0BU	// scan limit	扫描界限寄存器
#define MAX7219_REG_INTENSITY		0x0AU	// intensity	亮度寄存器
#define MAX7219_REG_TEST			0x0FU	// test			测试寄存器

#define MAX7219_DEFAULT_INTENSITY	0x03U	// 默认亮度

typedef struct {
	#if SYS_EN_FREERTOS
		SemaphoreHandle_t mutex_lock;
	#endif /* SYS_EN_FREERTOS */
	void		*hspi;
	uint8_t		*data;
	uint8_t		count;
	uint8_t		intensity;
	uint8_t		inited;
} MAX7219_Handle_t;

MAX7219_Status_t MAX7219_Init(MAX7219_Handle_t *handle, void *hspi, uint8_t count);
MAX7219_Status_t MAX7219_DeInit(MAX7219_Handle_t *handle);

MAX7219_Status_t MAX7219_Lock(MAX7219_Handle_t *handle);
MAX7219_Status_t MAX7219_Unlock(MAX7219_Handle_t *handle);

MAX7219_Status_t MAX7219_WriteCmd(MAX7219_Handle_t *handle, uint8_t reg_addr, uint8_t byte);
MAX7219_Status_t MAX7219_WriteData(MAX7219_Handle_t *handle, uint8_t *data, uint16_t len);

MAX7219_Status_t MAX7219_SetIntensity(MAX7219_Handle_t *handle, uint8_t intensity);
MAX7219_Status_t MAX7219_Clear(MAX7219_Handle_t *handle);

MAX7219_Status_t MAX7219_Refresh(MAX7219_Handle_t *handle);

MAX7219_Status_t MAX7219_DarwChar(MAX7219_Handle_t *handle, int16_t x, int16_t y, uint8_t ch, uint8_t *font);

#ifdef __cplusplus
}
#endif

#endif  /* __MAX7219_H */