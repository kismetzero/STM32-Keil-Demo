#include "AHT20_i2c_bus.h"
#include "delay.h"
#include "USART1.h"

// AHT20 默认 I2C 地址
#define AHT20_DEFAULT_I2C_ADDR	0x38	// 0011 1000

static float AHT20_GetTemperature(uint8_t *data) {
	uint32_t raw_temperature;
	raw_temperature = (((data[3] & 0x0F) << 16) | (data[4] << 8)) | data[5];
	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
}

static float AHT20_GetHumidity(uint8_t *data) {
	uint32_t raw_humidity;
	raw_humidity = ((data[1] << 12) | (data[2] << 4)) | (data[3] >> 4);
	return ((raw_humidity * 100.0f) / (1 << 20));
}

AHT20_Status_t AHT20_Init(AHT20_Handle_t *dev) {
	if (dev == NULL) { return AHT20_ERR_INVALID_PARAM; }
	if (dev->hi2c == NULL) { return AHT20_ERR_I2C_ERR; }
	if (dev->i2c_addr == 0) { dev->i2c_addr = AHT20_DEFAULT_I2C_ADDR; }
	i2c_bus_status_t i2c_ret;
	static const uint8_t AHT20_InitCommand[] = {0xBE, 0x08, 0x00};
	
	delay_ms(50);
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, AHT20_InitCommand, 3);
	if (i2c_ret != I2C_BUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	delay_ms(50);
	
	i2c_ret = i2c_master_receive(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	// 检查校准位
	if (!(dev->raw_data[0] & (1 << 3))) { return AHT20_ERR_CAL; }
	
	dev->temperature = AHT20_GetTemperature(dev->raw_data);
	dev->humidity = AHT20_GetHumidity(dev->raw_data);
	return AHT20_OK;
}

AHT20_Status_t AHT20_Reset(AHT20_Handle_t *dev) {
	if (!dev) { return AHT20_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return AHT20_ERR_I2C_ERR; }
	i2c_bus_status_t i2c_ret;
	static const uint8_t AHT20_ResetCommand[] = {0xBA};
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, AHT20_ResetCommand, 1);
	if (i2c_ret != I2C_BUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	return AHT20_OK;
}

AHT20_Status_t AHT20_Measure(AHT20_Handle_t *dev) {
	if (!dev) { return AHT20_ERR_INVALID_PARAM; }
	if (!dev->hi2c) { return AHT20_ERR_I2C_ERR; }
	i2c_bus_status_t i2c_ret;
	static const uint8_t AHT20_MeasureCommand[] = {0xAC, 0x33, 0x00};
	
	i2c_ret = i2c_master_transmit(dev->hi2c, dev->i2c_addr, AHT20_MeasureCommand, 3);
	if (i2c_ret != I2C_BUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	delay_ms(80);
	
	i2c_ret = i2c_master_receive(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	// 检查是否忙
	if (dev->raw_data[0] & (1 << 7)) { return AHT20_ERR_BUSY; }
	
	dev->temperature = AHT20_GetTemperature(dev->raw_data);
	dev->humidity = AHT20_GetHumidity(dev->raw_data);
	return AHT20_OK;
}
