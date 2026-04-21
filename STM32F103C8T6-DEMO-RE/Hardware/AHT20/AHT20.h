#ifndef __AHT20_H
#define __AHT20_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

// AHT20 默认 I2C 地址
#define AHT20_DEFAULT_I2C_ADDR 0x38 // 0011 1000

// 定义返回状态码
typedef enum {
    AHT20_STATUS_OK = 0,
	AHT20_STATUS_ERR,
	AHT20_STATUS_ERR_INVALID_PARAM,
	AHT20_STATUS_ERR_NO_INIT,
	AHT20_STATUS_ERR_I2C_ERR,
	AHT20_STATUS_ERR_BUSY,
	AHT20_STATUS_ERR_CAL,
} AHT20_Status_t;

// 定义 AHT20 设备句柄结构体
typedef struct AHT20_Handle_s AHT20_Handle_t;
struct AHT20_Handle_s {
	float		temperature;
	float		humidity;
	void		*hi2c;
	uint8_t		i2c_addr;
	uint8_t		inited;
};

// 设备初始化
AHT20_Status_t AHT20_Init(AHT20_Handle_t *handle, void *hi2c, uint8_t i2c_addr);
AHT20_Status_t AHT20_Reset(AHT20_Handle_t *handle);			// 软复位
AHT20_Status_t AHT20_Measure(AHT20_Handle_t *handle);		// 开启测量并读取
AHT20_Status_t AHT20_StartMeasure(AHT20_Handle_t *handle);	// 开启测量
AHT20_Status_t AHT20_ReadData(AHT20_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */