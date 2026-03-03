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
	AHT20_ERR_BUSY,
	AHT20_ERR_CAL,
	AHT20_ERR_I2C_ERR
} AHT20_status_t;

extern

AHT20_status_t AHT20_Init(i2c_if_handle_t *hi2c);		// 设备初始化
AHT20_status_t AHT20_Reset(void);						// 软复位
AHT20_status_t AHT20_Measure(void);						// 开启测量并读取
float AHT20_GetTemperature(void);
float AHT20_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */