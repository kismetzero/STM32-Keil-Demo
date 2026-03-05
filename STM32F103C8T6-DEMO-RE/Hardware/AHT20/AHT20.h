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
	AHT20_ERR_I2C_ERR,
	AHT20_ERR_INVALID_PARAM
} AHT20_status_t;

// 定义 AHT20 设备句柄结构体
typedef struct {
	i2c_if_handle_t *hi2c;
	uint8_t i2c_addr;
	uint8_t raw_data[6];
	float temperature;
	float humidity;
} AHT20_handle_t;

AHT20_status_t AHT20_Init(AHT20_handle_t *dev);			// 设备初始化
AHT20_status_t AHT20_Reset(AHT20_handle_t *dev);		// 软复位
AHT20_status_t AHT20_Measure(AHT20_handle_t *dev);		// 开启测量并读取

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */