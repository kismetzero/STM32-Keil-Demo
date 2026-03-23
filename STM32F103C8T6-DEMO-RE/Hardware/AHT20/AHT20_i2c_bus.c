#include "AHT20.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "AHT20"
#include "elog.h"

static const uint8_t AHT20_InitCommand[]		=	{0xBE, 0x08, 0x00};
static const uint8_t AHT20_ResetCommand			=	0xBA;
static const uint8_t AHT20_MeasureCommand[]		=	{0xAC, 0x33, 0x00};

AHT20_Status_t AHT20_Init(AHT20_Handle_t *dev, void *hi2c, uint8_t i2c_addr) {
	if (dev == NULL) {
		log_e("AHT20_Init: Fail! dev == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (hi2c == NULL) {
		log_e("AHT20_Init: Fail! hi2c == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	dev->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("AHT20_Init: Info Using default addr 0x%02X", AHT20_DEFAULT_I2C_ADDR);
		i2c_addr = AHT20_DEFAULT_I2C_ADDR;
	}
	dev->i2c_addr = i2c_addr;
	i2c_bus_status_t i2c_ret;
	delay_ms(50);
	i2c_ret = i2c_write_bytes(dev->hi2c, dev->i2c_addr, AHT20_InitCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Write Init Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	delay_ms(50);
	i2c_ret = i2c_read_bytes(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	// 检查校准位
	if (!(dev->raw_data[0] & (1 << 3))) {
		log_e("AHT20_Init: Fail! @ Not Calibrated (Bit 3 is 0)");
		return AHT20_STATUS_ERR_CAL;
	}
	float temperature = AHT20_CalcTemperature(dev->raw_data);
	float humidity = AHT20_CalcHumidity(dev->raw_data);
	if (humidity < 0) {
		log_w("AHT20_Init: humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("AHT20_Init: humidity > 100");
		humidity = 100;
	}
	dev->temperature = temperature;
	dev->humidity = humidity;
	log_i("AHT20_Init: Success! temperature=%f humidity=%f", temperature, humidity);
	return AHT20_STATUS_OK;
}

AHT20_Status_t AHT20_Reset(AHT20_Handle_t *dev) {
	if (dev == NULL) {
		log_e("AHT20_Reset: Fail! dev == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("AHT20_Reset: Fail! dev->hi2c == NULL");
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_byte(dev->hi2c, dev->i2c_addr, AHT20_ResetCommand);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Reset: Fail! @ Write Reset Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	return AHT20_STATUS_OK;
}

AHT20_Status_t AHT20_Measure(AHT20_Handle_t *dev) {
	if (dev == NULL) {
		log_e("AHT20_Measure: Fail! dev == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("AHT20_Measure: Fail! dev->hi2c == NULL");
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_bytes(dev->hi2c, dev->i2c_addr, AHT20_MeasureCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail! @ Write Measure Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	delay_ms(80);
	i2c_ret = i2c_read_bytes(dev->hi2c, dev->i2c_addr, dev->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	// 检查是否忙
	if (dev->raw_data[0] & (1 << 7)) {
		log_e("AHT20_Measure: Fail! @ Dev Busy (Bit 7 is 1)");
		return AHT20_STATUS_ERR_BUSY;
	}
	float temperature = AHT20_CalcTemperature(dev->raw_data);
	float humidity = AHT20_CalcHumidity(dev->raw_data);
	if (humidity < 0) {
		log_w("AHT20_Measure: humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("AHT20_Measure: humidity > 100");
		humidity = 100;
	}
	dev->temperature = temperature;
	dev->humidity = humidity;
	log_i("AHT20_Measure: Success! temperature=%f, humidity=%f", temperature, humidity);
	return AHT20_STATUS_OK;
}
