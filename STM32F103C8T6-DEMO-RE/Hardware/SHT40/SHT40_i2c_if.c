#include "SHT40.h"
#include "i2c_if.h"
#include "delay.h"

// AHT20 I2C 地址
#define SHT40_I2C_ADDR		0x44	// 0100 0100

uint8_t SHT40_RawData[6];
static i2c_if_handle_t *g_hi2c;
static SHT40_Repeatability_t SHT40_Rep = SHT40_HIGH_REP;
static const uint8_t SHT40_MeasureCommand[] = {0xFD, 0xF6, 0xE0};
static const uint8_t SHT40_MeasureDelay[] = {16, 8, 6};

static uint8_t SHT40_CalcCRC8(const uint8_t *data, uint8_t length) {
	uint8_t crc = 0xFF; // 初始值
    uint8_t i, j;

    for (i = 0; i < 6; i++) {
        crc ^= SHT40_RawData[i]; // 异或当前字节
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) {
                // 如果最高位为 1，左移后异或多项式 0x31 (0011 0001)
                crc = (crc << 1) ^ 0x31;
            } else {
                // 如果最高位为 0，仅左移
                crc = (crc << 1);
            }
        }
    }
	return crc;
}

static bool SHT40_CheckCRC(void) {
	bool res;
	res = SHT40_CalcCRC8(SHT40_RawData, 2) == SHT40_RawData[2];
	res = SHT40_CalcCRC8(&SHT40_RawData[3], 2) == SHT40_RawData[5];
	return res;
}

SHT40_status_t SHT40_Init(i2c_if_handle_t *hi2c) {
	if (!hi2c) { return SHT40_ERR_I2C_ERR; }
	g_hi2c = hi2c;
	return SHT40_OK;
}

SHT40_status_t SHT40_Reset(void) {
	i2c_if_status_t i2c_if_ret;
	static const uint8_t SHT40_ResetCommand[] = {0x94};
	
	i2c_if_ret = i2c_master_transmit(g_hi2c, SHT40_I2C_ADDR, SHT40_ResetCommand, 1);
	if (i2c_if_ret != I2C_IF_OK) { return SHT40_ERR_I2C_ERR; }
	
	return SHT40_OK;
}

SHT40_status_t SHT40_Measure(void) {
	i2c_if_status_t i2c_if_ret;
	
	i2c_if_ret = i2c_master_transmit(g_hi2c, SHT40_I2C_ADDR, &SHT40_MeasureCommand[SHT40_Rep], 1);
	if (i2c_if_ret != I2C_IF_OK) { return SHT40_ERR_I2C_ERR; }
	
	delay_ms(SHT40_MeasureDelay[SHT40_Rep]);
	
	i2c_if_ret = i2c_master_receive(g_hi2c, SHT40_I2C_ADDR, SHT40_RawData, 6);
	if (i2c_if_ret != I2C_IF_OK) { return SHT40_ERR_I2C_ERR; }
	
	if (!SHT40_CheckCRC()) { return SHT40_ERR_CRC; }
	
	return SHT40_OK;
}

float SHT40_GetTemperature(void) {
	uint32_t raw_temperature;
	raw_temperature = (SHT40_RawData[0]  << 8) | SHT40_RawData[1];
	return -45.0f + 175.0f * raw_temperature / 65535.0f;
}

float SHT40_GetHumidity(void) {
	uint32_t raw_humidity;
	raw_humidity = (SHT40_RawData[3] << 8) | SHT40_RawData[4];
	return -6.0f + 125.0f * raw_humidity / 65535.0f;
}
