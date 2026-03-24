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
	AHT20_STATUS_ERR_BUSY,
	AHT20_STATUS_ERR_CAL,
	AHT20_STATUS_ERR_I2C_ERR,
	AHT20_STATUS_ERR_INVALID_PARAM
} AHT20_Status_t;

// 定义 AHT20 设备句柄结构体
typedef struct {
	void *hi2c;
	uint8_t i2c_addr;
	uint8_t raw_data[6];
	float temperature;
	float humidity;
} AHT20_Handle_t;

// 设备初始化
AHT20_Status_t AHT20_Init(AHT20_Handle_t *dev, void *hi2c, uint8_t i2c_addr);
AHT20_Status_t AHT20_Reset(AHT20_Handle_t *dev);		// 软复位
AHT20_Status_t AHT20_Measure(AHT20_Handle_t *dev);		// 开启测量并读取

static inline float AHT20_CalcTemperature(const uint8_t *data) {
//	uint32_t raw_temperature;
//	raw_temperature = (((data[3] & 0x0F) << 16) | (data[4] << 8)) | data[5];
//	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
	uint32_t raw_temperature =	(uint32_t)(data[3] & 0x0F) << 16 |
								(uint32_t)data[4] << 8 |
								(uint32_t)data[5];
	return (raw_temperature * 0.00019073486f) - 50.0f;
}

static inline float AHT20_CalcHumidity(const uint8_t *data) {
//	uint32_t raw_humidity;
//	raw_humidity = ((data[1] << 12) | (data[2] << 4)) | (data[3] >> 4);
//	return ((raw_humidity * 100.0f) / (1 << 20));
	uint32_t raw_humidity = (uint32_t)data[1] << 12 |
							(uint32_t)data[2] << 4 |
							(uint32_t)data[3] >> 4;
	return raw_humidity * 0.00009536743f;
}

#ifdef __cplusplus
}
#endif

#endif  /* __AHT20_H */