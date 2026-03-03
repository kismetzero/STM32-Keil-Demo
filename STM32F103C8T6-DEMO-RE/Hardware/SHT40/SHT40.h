#ifndef __SHT40_H
#define __SHT40_H

#include "stdint.h"
#include "i2c_if.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    SHT40_OK = 0,
	SHT40_ERR_BUSY,
	SHT40_ERR_CRC,
	SHT40_ERR_I2C_ERR
} SHT40_status_t;

typedef enum {
    SHT40_HIGH_REP = 0,
    SHT40_MED_REP,
    SHT40_LOW_REP
} SHT40_Repeatability_t;

SHT40_status_t SHT40_Init(i2c_if_handle_t *hi2c);		// 设备初始化
SHT40_status_t SHT40_Reset(void);						// 软复位
SHT40_status_t SHT40_Measure(void);						// 开启测量并读取
float SHT40_GetTemperature(void);
float SHT40_GetHumidity(void);

#ifdef __cplusplus
}
#endif

#endif  /* __SHT40_H */