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
	SHT40_ERR_I2C_ERR,
	SHT40_ERR_INVALID_PARAM
} SHT40_Status_t;

// 测量精度
typedef enum {
    SHT40_REP_HIGH = 0,
    SHT40_REP_MED,
    SHT40_REP_LOW
} SHT40_Repeatability_t;

// 内置加热器
typedef enum {
    SHT40_HEATER_200MW1S = 0,
    SHT40_HEATER_200MW100MS,
	SHT40_HEATER_110MW1S,
    SHT40_HEATER_110MW100MS,
	SHT40_HEATER_20MW1S,
    SHT40_HEATER_20MW100MS
} SHT40_Heater_t;

// 定义 SHT40 设备句柄结构体
typedef struct {
	i2c_if_handle_t *hi2c;
	uint8_t i2c_addr;
	uint8_t raw_data[6];
	float temperature;
	float humidity;
	SHT40_Repeatability_t repeatability;
} SHT40_Handle_t;

SHT40_Status_t SHT40_Init(SHT40_Handle_t *dev);			// 设备初始化
SHT40_Status_t SHT40_Reset(SHT40_Handle_t *dev);		// 软复位
SHT40_Status_t SHT40_Measure(SHT40_Handle_t *dev);		// 开启测量并读取

// 开启内置加热器测量并读取（！！慎用！！）
SHT40_Status_t SHT40_HeaterMeasure(SHT40_Handle_t *dev, SHT40_Heater_t heater);	

#ifdef __cplusplus
}
#endif

#endif  /* __SHT40_H */