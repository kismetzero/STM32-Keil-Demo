#ifndef __BMP280_H
#define __BMP280_H

#include "stdint.h"
#include "i2c_if.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    BMP280_OK = 0,
	BMP280_ERR_BUSY,
	BMP280_ERR_I2C_ERR,
	BMP280_ERR_SPI_ERR,
	BMP280_ERR_INVALID_PARAM
} BMP280_Status_t;

// 校准参数结构体 (从芯片读取的补偿系数)
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_CalibData_t;

// 定义 BMP280 设备句柄结构体
typedef struct {
	i2c_if_handle_t *hi2c;
	uint8_t i2c_addr;
} BMP280_Handle_t;

void BMP280_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __BMP280_H */