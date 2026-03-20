#include "AHT20.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "aht20"
#include "elog.h"

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
	if (dev == NULL) {
		log_e("AHT20_Init: Fail! dev == NULL");
		return AHT20_ERR_INVALID_PARAM;
	}
	
	if (dev->hi2c == NULL) {
		log_e("AHT20_Init: Fail! dev->hi2c == NULL");
		return AHT20_ERR_I2C_ERR;
	}
	
	if (dev->i2c_addr == 0) {
		log_i("AHT20_Init: Info Using default addr 0x%02X", AHT20_DEFAULT_I2C_ADDR);
		dev->i2c_addr = AHT20_DEFAULT_I2C_ADDR;
	}
	
	i2c_bus_status_t i2c_ret;
	
	static const uint8_t AHT20_InitCommand[] = {0xBE, 0x08, 0x00};
	
	delay_ms(50);
	
	i2c_ret = i2c_write_bytes(dev->hi2c, dev->i2c_addr, AHT20_InitCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Write Init Cmd Fail(Code: %d)", i2c_ret);
		return AHT20_ERR_I2C_ERR;
	}
	
	delay_ms(50);
	
	i2c_ret = i2c_read_bytes(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Read Data Fail(Code: %d)", i2c_ret);
		return AHT20_ERR_I2C_ERR;
	}
	
	// 检查校准位
	if (!(dev->raw_data[0] & (1 << 3))) {
		log_e("AHT20_Init: Fail @ Not Calibrated (Bit 3 is 0)");
		return AHT20_ERR_CAL;
	}
	
	dev->temperature = AHT20_GetTemperature(dev->raw_data);
	dev->humidity = AHT20_GetHumidity(dev->raw_data);
	
	log_d("AHT20_Init: Success");
	return AHT20_OK;
}

AHT20_Status_t AHT20_Reset(AHT20_Handle_t *dev) {
	if (dev == NULL) {
		log_e("AHT20_Reset: Fail dev == NULL");
		return AHT20_ERR_INVALID_PARAM;
	}
	
	if (dev->hi2c == NULL) {
		log_e("AHT20_Reset: Fail dev->hi2c == NULL");
		return AHT20_ERR_I2C_ERR;
	}
	
	i2c_bus_status_t i2c_ret;
	static const uint8_t AHT20_ResetCommand[] = {0xBA};
	
	i2c_ret = i2c_write_bytes(dev->hi2c, dev->i2c_addr, AHT20_ResetCommand, 1);
	if (i2c_ret != I2C_BUS_STATUS_OK) { return AHT20_ERR_I2C_ERR; }
	
	return AHT20_OK;
}

AHT20_Status_t AHT20_Measure(AHT20_Handle_t *dev) {
	if (dev == NULL) {
		log_e("AHT20_Measure: Fail dev == NULL");
		return AHT20_ERR_INVALID_PARAM;
	}
	
	if (dev->hi2c == NULL) {
		log_e("AHT20_Measure: Fail dev->hi2c == NULL");
		return AHT20_ERR_I2C_ERR;
	}
	
	i2c_bus_status_t i2c_ret;
	static const uint8_t AHT20_MeasureCommand[] = {0xAC, 0x33, 0x00};
	
	i2c_ret = i2c_write_bytes(dev->hi2c, dev->i2c_addr, AHT20_MeasureCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail @ Write Measure Cmd Fail(Code: %d)", i2c_ret);
		return AHT20_ERR_I2C_ERR;
	}
	
	delay_ms(80);
	
	i2c_ret = i2c_read_bytes(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail @ Read Data Fail(Code: %d)", i2c_ret);
		return AHT20_ERR_I2C_ERR;
	}
	
	// 检查是否忙
	if (dev->raw_data[0] & (1 << 7)) {
		log_e("AHT20_Measure: Fail @ Dev Busy (Bit 7 is 0)");
		return AHT20_ERR_BUSY;
	}
	
	dev->temperature = AHT20_GetTemperature(dev->raw_data);
	dev->humidity = AHT20_GetHumidity(dev->raw_data);
	
	log_d("AHT20_Measure: Success Temp=%f, Humid=%f", dev->temperature, dev->humidity);
	return AHT20_OK;
}
