#include "AHT20.h"
#include "i2c_if.h"
#include "delay.h"

// AHT20 I2C 地址
#define AHT20_I2C_ADDR		0x38	// 0011 1000

uint8_t AHT20_RawData[6];
static i2c_if_handle_t *g_hi2c;

AHT20_status_t AHT20_Init(i2c_if_handle_t *hi2c) {
	if (!hi2c) { return AHT20_ERR_I2C_ERR; }
	g_hi2c = hi2c;
	i2c_if_status_t i2c_if_ret;
	static const uint8_t AHT20_InitCommand[] = {0xBE, 0x08, 0x00};
	
	delay_ms(50);
	
	i2c_if_ret = i2c_master_transmit(g_hi2c, AHT20_I2C_ADDR, AHT20_InitCommand, 3);
	if (i2c_if_ret != I2C_IF_OK) { return AHT20_ERR_I2C_ERR; }
	
	delay_ms(50);
	
	i2c_if_ret = i2c_master_receive(g_hi2c, AHT20_I2C_ADDR, AHT20_RawData, 6);
	if (i2c_if_ret != I2C_IF_OK) { return AHT20_ERR_I2C_ERR; }
	
	// 检查校准位
	if (!(AHT20_RawData[0] & (1 << 3))) { return AHT20_ERR_CAL; }
	return AHT20_OK;
}

AHT20_status_t AHT20_Reset(void) {
	i2c_if_status_t i2c_if_ret;
	static const uint8_t AHT20_ResetCommand[] = {0xBA};
	
	i2c_if_ret = i2c_master_transmit(g_hi2c, AHT20_I2C_ADDR, AHT20_ResetCommand, 1);
	if (i2c_if_ret != I2C_IF_OK) { return AHT20_ERR_I2C_ERR; }
	
	return AHT20_OK;
}

AHT20_status_t AHT20_Measure(void) {
	i2c_if_status_t i2c_if_ret;
	static const uint8_t AHT20_MeasureCommand[] = {0xAC, 0x33, 0x00};
	
	i2c_if_ret = i2c_master_transmit(g_hi2c, AHT20_I2C_ADDR, AHT20_MeasureCommand, 3);
	if (i2c_if_ret != I2C_IF_OK) { return AHT20_ERR_I2C_ERR; }
	
	delay_ms(80);
	
	i2c_if_ret = i2c_master_receive(g_hi2c, AHT20_I2C_ADDR, AHT20_RawData, 6);
	if (i2c_if_ret != I2C_IF_OK) { return AHT20_ERR_I2C_ERR; }
	
	// 检查是否忙
	if (AHT20_RawData[0] & (1 << 7)) { return AHT20_ERR_BUSY; }
	return AHT20_OK;
}

float AHT20_GetTemperature(void) {
	uint32_t raw_temperature;
	raw_temperature = (((AHT20_RawData[3] & 0x0F) << 16) | (AHT20_RawData[4] << 8)) | AHT20_RawData[5];
	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
}

float AHT20_GetHumidity(void) {
	uint32_t raw_humidity;
	raw_humidity = ((AHT20_RawData[1] << 12) | (AHT20_RawData[2] << 4)) | (AHT20_RawData[3] >> 4);
	return ((raw_humidity * 100.0f) / (1 << 20));
}
