#include "AHT20.h"
#include "i2c_if.h"
#include "delay.h"

// AHT20 I2C 地址
#define AHT20_I2C_ADDR		0x38	// 0011 1000

static uint8_t AHT20_data[6];
static i2c_if_handle_t *g_hi2c;

AHT20_status_t AHT20_Init(i2c_if_handle_t *hi2c) {
	g_hi2c = hi2c;
	i2c_if_status_t ret;
	delay_ms(50);
	
	ret = g_hi2c->start(g_hi2c->user_data);
	g_hi2c->write_byte(g_hi2c->user_data, AHT20_I2C_ADDR << 1);
	g_hi2c->write_byte(g_hi2c->user_data, 0xBE);
	g_hi2c->write_byte(g_hi2c->user_data, 0x08);
	g_hi2c->write_byte(g_hi2c->user_data, 0x00);
	g_hi2c->stop(g_hi2c->user_data);
	
	delay_ms(50);
	
	g_hi2c->start(g_hi2c->user_data);
	g_hi2c->write_byte(g_hi2c->user_data, (AHT20_I2C_ADDR << 1) | 1);
	for (uint8_t i = 0; i < 6; i++) {
		if (i < 5) { g_hi2c->read_byte(g_hi2c->user_data, &AHT20_data[i], 1); }
		else { g_hi2c->read_byte(g_hi2c->user_data, &AHT20_data[i], 0); }
	}
	g_hi2c->stop(g_hi2c->user_data);
	
	// 检查校准位
	if (!(AHT20_data[0] & (1 << 3))) { return AHT20_CAL_ERRO; }
	
	return AHT20_OK;
}

AHT20_status_t AHT20_Reset(void) {
	i2c_if_status_t ret;
	
	ret = g_hi2c->start(g_hi2c->user_data);
	g_hi2c->write_byte(g_hi2c->user_data, AHT20_I2C_ADDR << 1);
	g_hi2c->write_byte(g_hi2c->user_data, 0xBA);
	g_hi2c->stop(g_hi2c->user_data);
	
	return AHT20_OK;
}

AHT20_status_t AHT20_Read(void) {
	i2c_if_status_t ret;
	
	ret = g_hi2c->start(g_hi2c->user_data);
	g_hi2c->write_byte(g_hi2c->user_data, AHT20_I2C_ADDR << 1);
	g_hi2c->write_byte(g_hi2c->user_data, 0xAC);
	g_hi2c->write_byte(g_hi2c->user_data, 0x33);
	g_hi2c->write_byte(g_hi2c->user_data, 0x00);
	g_hi2c->stop(g_hi2c->user_data);
	
	delay_ms(80);
	
	ret = g_hi2c->start(g_hi2c->user_data);
	g_hi2c->write_byte(g_hi2c->user_data, (AHT20_I2C_ADDR << 1) | 1);
	for (uint8_t i = 0; i < 6; i++) {
		if (i < 5) { g_hi2c->read_byte(g_hi2c->user_data, &AHT20_data[i], 1); }
		else { g_hi2c->read_byte(g_hi2c->user_data, &AHT20_data[i], 0); }
	}
	g_hi2c->stop(g_hi2c->user_data);
	
	// 检查是否忙
	if (AHT20_data[0] & (1 << 7)) { return AHT20_BUSY; }
	
	return AHT20_OK;
}

float AHT20_GetTemperature(void) {
	uint32_t raw_temperature;
	raw_temperature = (((AHT20_data[3] & 0x0F) << 16) | (AHT20_data[4] << 8)) | AHT20_data[5];
	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
}

float AHT20_GetHumidity(void) {
	uint32_t raw_humidity;
	raw_humidity = ((AHT20_data[1] << 12) | (AHT20_data[2] << 4)) | (AHT20_data[3] >> 4);
	return ((raw_humidity * 100.0f) / (1 << 20));
}
