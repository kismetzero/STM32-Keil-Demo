#ifndef __AHT20_H
#define __AHT20_H

#include "stdint.h"
#include "i2c_if.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    AHT20_OK = 0,
	AHT20_BUSY,
	AHT20_CAL_ERRO,
	AHT20_I2C_ERRO
} AHT20_status_t;

AHT20_status_t AHT20_Init(i2c_if_handle_t *hi2c);
AHT20_status_t AHT20_Read(void);						// 开启测量并读取 return 0 为成功
AHT20_status_t AHT20_Reset(void);						// 软复位 return 0 为成功
float AHT20_GetTemperature(void);
float AHT20_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */