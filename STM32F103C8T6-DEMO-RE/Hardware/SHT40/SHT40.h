#ifndef __SHT40_H
#define __SHT40_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

// AHT20 默认 I2C 地址
#define SHT40_DEFAULT_I2C_ADDR	0x44	// 0100 0100

// 定义返回状态码
typedef enum {
    SHT40_STATUS_OK = 0,
	SHT40_STATUS_ERR_INVALID_PARAM,
	SHT40_STATUS_ERR_I2C_ERR,
	SHT40_STATUS_ERR_BUSY,
	SHT40_STATUS_ERR_CRC,
} SHT40_Status_t;

// 测量精度
typedef enum {
	SHT40_REP_DEFAULT	= 0,
    SHT40_REP_HIGH	= 0,
    SHT40_REP_MED	= 1,
    SHT40_REP_LOW	= 2
} SHT40_Repeatability_t;

// 内置加热器
typedef enum {
    SHT40_HEATER_200MW1S		= 0,
    SHT40_HEATER_200MW100MS		= 1,
	SHT40_HEATER_110MW1S		= 2,
    SHT40_HEATER_110MW100MS		= 3,
	SHT40_HEATER_20MW1S			= 4,
    SHT40_HEATER_20MW100MS		= 5
} SHT40_Heater_t;

// 定义 SHT40 设备句柄结构体
typedef struct SHT40_Handle_s SHT40_Handle_t;
struct SHT40_Handle_s {
	float temperature;
	float humidity;
	SHT40_Repeatability_t repeatability;
	void *hi2c;
	uint8_t i2c_addr;
};

// 设备初始化
SHT40_Status_t SHT40_Init(SHT40_Handle_t *handle, void *hi2c, uint8_t i2c_addr, SHT40_Repeatability_t rep);
SHT40_Status_t SHT40_Reset(SHT40_Handle_t *handle);			// 软复位
SHT40_Status_t SHT40_Measure(SHT40_Handle_t *handle);		// 开启测量并读取
SHT40_Status_t SHT40_SetRepeatability(SHT40_Handle_t *handle, SHT40_Repeatability_t rep);

// 开启内置加热器测量并读取（！！慎用！！）
SHT40_Status_t SHT40_HeaterMeasure(SHT40_Handle_t *handle, SHT40_Heater_t heater);	

#ifdef __cplusplus
}
#endif

#endif  /* __SHT40_H */