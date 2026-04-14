#include "AHT20.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "AHT20"
#include "elog.h"

static const uint8_t AHT20_InitCommand[]		=	{0xBE, 0x08, 0x00};
static const uint8_t AHT20_ResetCommand			=	0xBA;
static const uint8_t AHT20_MeasureCommand[]		=	{0xAC, 0x33, 0x00};

static inline float AHT20_CalcTemperature(const uint8_t *data) {
	uint32_t raw_temperature =	(uint32_t)(data[3] & 0x0F) << 16 |
								(uint32_t)data[4] << 8 |
								(uint32_t)data[5];
//	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
	return (raw_temperature * 0.00019073486f) - 50.0f;
}

static inline float AHT20_CalcHumidity(const uint8_t *data) {
	uint32_t raw_humidity = (uint32_t)data[1] << 12 |
							(uint32_t)data[2] << 4 |
							(uint32_t)data[3] >> 4;
//	return ((raw_humidity * 100.0f) / (1 << 20));
	return raw_humidity * 0.00009536743f;
}

AHT20_Status_t AHT20_Init(AHT20_Handle_t *handle, void *hi2c, uint8_t i2c_addr) {
	if (handle == NULL) {
		log_e("AHT20_Init: Fail! handle == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (hi2c == NULL) {
		log_e("AHT20_Init: Fail! hi2c == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	handle->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("AHT20_Init: Info Using default addr 0x%02X", AHT20_DEFAULT_I2C_ADDR);
		i2c_addr = AHT20_DEFAULT_I2C_ADDR;
	}
	handle->i2c_addr = i2c_addr;
	i2c_bus_status_t i2c_ret;
	delay_ms(50);
	i2c_ret = i2c_write_bytes(handle->hi2c, handle->i2c_addr, AHT20_InitCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Write Init Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	delay_ms(50);
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, handle->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Init: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	// 检查校准位
	if (!(handle->raw_data[0] & (1 << 3))) {
		log_e("AHT20_Init: Fail! @ Not Calibrated (Bit 3 is 0)");
		return AHT20_STATUS_ERR_CAL;
	}
	float temperature = AHT20_CalcTemperature(handle->raw_data);
	float humidity = AHT20_CalcHumidity(handle->raw_data);
	if (humidity < 0) {
		log_w("AHT20_Init: Warning! humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("AHT20_Init: Warning! humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_i("AHT20_Init: Success! temperature=%f humidity=%f", temperature, humidity);
	return AHT20_STATUS_OK;
}

AHT20_Status_t AHT20_Reset(AHT20_Handle_t *handle) {
	if (handle == NULL) {
		log_e("AHT20_Reset: Fail! handle == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hi2c == NULL) {
		log_e("AHT20_Reset: Fail! handle->hi2c == NULL");
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, AHT20_ResetCommand);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Reset: Fail! @ Write Reset Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	return AHT20_STATUS_OK;
}

AHT20_Status_t AHT20_Measure(AHT20_Handle_t *handle) {
	if (handle == NULL) {
		log_e("AHT20_Measure: Fail! handle == NULL");
		return AHT20_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hi2c == NULL) {
		log_e("AHT20_Measure: Fail! handle->hi2c == NULL");
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_bytes(handle->hi2c, handle->i2c_addr, AHT20_MeasureCommand, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail! @ Write Measure Cmd Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	delay_ms(80);
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, handle->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("AHT20_Measure: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return AHT20_STATUS_ERR_I2C_ERR;
	}
	// 检查是否忙
	if (handle->raw_data[0] & (1 << 7)) {
		log_e("AHT20_Measure: Fail! @ Dev Busy (Bit 7 is 1)");
		return AHT20_STATUS_ERR_BUSY;
	}
	float temperature = AHT20_CalcTemperature(handle->raw_data);
	float humidity = AHT20_CalcHumidity(handle->raw_data);
	if (humidity < 0) {
		log_w("AHT20_Measure: Warning! humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("AHT20_Measure: Warning! humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_i("AHT20_Measure: Success! temperature=%f, humidity=%f", temperature, humidity);
	return AHT20_STATUS_OK;
}
