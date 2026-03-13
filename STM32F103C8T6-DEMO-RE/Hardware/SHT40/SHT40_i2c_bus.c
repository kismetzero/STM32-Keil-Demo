#include "SHT40_i2c_bus.h"
#include "delay.h"

// AHT20 默认 I2C 地址
#define SHT40_DEFAULT_I2C_ADDR	0x44	// 0100 0100

static const uint8_t SHT40_MeasureCommand[] = {0xFD, 0xF6, 0xE0};
static const uint8_t SHT40_MeasureDelay[] = {16, 8, 6};
static const uint8_t SHT40_HeaterMeasureCommand[] = {0x39, 0x32, 0x2F, 0x24, 0x1E, 0x15};
static const uint16_t SHT40_HeaterMeasureDelay[] = {1100, 200};

static uint8_t SHT40_CalcCRC8(const uint8_t *data, uint8_t length) {
	uint8_t crc = 0xFF; // 初始值
    uint8_t i, j;

    for (i = 0; i < length; i++) {
        crc ^= data[i]; // 异或当前字节
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

static bool SHT40_CheckCRC(uint8_t *data) {
	return (SHT40_CalcCRC8(data, 2) == data[2]) &&
			(SHT40_CalcCRC8(&data[3], 2) == data[5]);
}

static float SHT40_GetTemperature(uint8_t *data) {
	uint32_t raw_temperature;
	raw_temperature = (data[0]  << 8) | data[1];
	return (-45.0f + 175.0f * raw_temperature / 65535.0f);
}

static float SHT40_GetHumidity(uint8_t *data) {
	uint32_t raw_humidity;
	raw_humidity = (data[3] << 8) | data[4];
	return (-6.0f + 125.0f * raw_humidity / 65535.0f);
}

SHT40_Status_t SHT40_Init(SHT40_Handle_t *dev) {
	if (!dev) { return SHT40_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return SHT40_ERR_I2C_ERR; }
	if (dev->i2c_addr == 0) { dev->i2c_addr = SHT40_DEFAULT_I2C_ADDR; }
	dev->repeatability = SHT40_REP_HIGH;
	return SHT40_OK;
}

SHT40_Status_t SHT40_Reset(SHT40_Handle_t *dev) {
	if (!dev) { return SHT40_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return SHT40_ERR_I2C_ERR; }
	i2c_bus_status_t i2c_ret;
	static const uint8_t SHT40_ResetCommand[] = {0x94};
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, SHT40_ResetCommand, 1);
	if (i2c_ret != I2C_BUS_OK) { return SHT40_ERR_I2C_ERR; }
	
	return SHT40_OK;
}

SHT40_Status_t SHT40_Measure(SHT40_Handle_t *dev) {
	if (!dev) { return SHT40_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return SHT40_ERR_I2C_ERR; }
	i2c_bus_status_t i2c_ret;
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, &SHT40_MeasureCommand[dev->repeatability], 1);
	if (i2c_ret != I2C_BUS_OK) { return SHT40_ERR_I2C_ERR; }
	
	delay_ms(SHT40_MeasureDelay[dev->repeatability]);
	
	i2c_ret = i2c_master_receive(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_OK) { return SHT40_ERR_I2C_ERR; }
	
	if (!SHT40_CheckCRC(dev->raw_data)) { return SHT40_ERR_CRC; }
	
	dev->temperature = SHT40_GetTemperature(dev->raw_data);
	dev->humidity = SHT40_GetHumidity(dev->raw_data);
	
	return SHT40_OK;
}

SHT40_Status_t SHT40_HeaterMeasure(SHT40_Handle_t *dev, SHT40_Heater_t heater) {
	if (!dev) { return SHT40_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return SHT40_ERR_I2C_ERR; }
	if (heater < 0 || heater > SHT40_HEATER_20MW100MS) { return SHT40_ERR_INVALID_PARAM; }
	return SHT40_ERR_BUSY;
	i2c_bus_status_t i2c_ret;
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, &SHT40_HeaterMeasureCommand[heater], 1);
	if (i2c_ret != I2C_BUS_OK) { return SHT40_ERR_I2C_ERR; }
	
	delay_ms(SHT40_HeaterMeasureDelay[(heater % 2)]);
	
	i2c_ret = i2c_master_receive(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_OK) { return SHT40_ERR_I2C_ERR; }
	
	if (!SHT40_CheckCRC(dev->raw_data)) { return SHT40_ERR_CRC; }
	
	dev->temperature = SHT40_GetTemperature(dev->raw_data);
	dev->humidity = SHT40_GetHumidity(dev->raw_data);
	
	return SHT40_OK;
}
