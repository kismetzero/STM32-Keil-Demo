#include "SHT40.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "SHT40"
#include "elog.h"

static const uint8_t SHT40_MeasureCommand[]			=	{0xFD, 0xF6, 0xE0};
static const uint8_t SHT40_MeasureDelay[]			=	{16, 8, 6};
static const uint8_t SHT40_ResetCommand				=	0x94;
static const uint8_t SHT40_HeaterMeasureCommand[]	=	{0x39, 0x32, 0x2F, 0x24, 0x1E, 0x15};
static const uint16_t SHT40_HeaterMeasureDelay[]	=	{1100, 200};

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

static inline bool SHT40_CheckCRC(const uint8_t *data) {
//	return (SHT40_CalcCRC8(data, 2) == data[2]) &&
//			(SHT40_CalcCRC8(&data[3], 2) == data[5]);
	if (SHT40_CalcCRC8(data, 2) != data[2]) { return false; }
	if (SHT40_CalcCRC8(&data[3], 2) != data[5]) { return false; }
	return true;
}

static inline float SHT40_CalcTemperature(const uint8_t *data) {
	uint16_t raw_temperature = (uint16_t)data[0] << 8 | (uint16_t)data[1];
//	return (-45.0f + 175.0f * raw_temperature / 65535.0f);
	return -45.0f + (raw_temperature * 0.00267032883f);
}

static inline float SHT40_CalcHumidity(const uint8_t *data) {
	uint16_t raw_humidity = (uint16_t)data[3] << 8 | (uint16_t)data[4];
//	return (-6.0f + 125.0f * raw_humidity / 65535.0f);
	return -6.0f + (raw_humidity * 0.00190737773f);
}

SHT40_Status_t SHT40_Init(SHT40_Handle_t *handle, void *hi2c, uint8_t i2c_addr, SHT40_Repeatability_t rep) {
	if (handle == NULL) {
		log_e("SHT40_Init: Fail! handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (hi2c == NULL) {
		log_e("SHT40_Init: Fail! hi2c == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	handle->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("SHT40_Init: Info Using default addr 0x%02X", SHT40_DEFAULT_I2C_ADDR);
		i2c_addr = SHT40_DEFAULT_I2C_ADDR;
	}
	handle->i2c_addr = i2c_addr;
	if (rep < SHT40_REP_DEFAULT || rep > SHT40_REP_LOW) {
		log_w("SHT40_Init: Warning! rep invalid! use SHT40_REP_DEFAULT");
		rep = SHT40_REP_DEFAULT;
	}
	handle->repeatability = rep;
	log_i("SHT40_Init: Success!");
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_Reset(SHT40_Handle_t *handle) {
	if (handle == NULL) {
		log_e("SHT40_Reset: Fail! handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hi2c == NULL) {
		log_e("SHT40_Reset: Fail! handle->hi2c == NULL");
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_ResetCommand);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("SHT40_Reset: Fail! @ Write Reset Cmd Fail (Code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_Measure(SHT40_Handle_t *handle) {
	if (handle == NULL) {
		log_e("SHT40_Measure: Fail! handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hi2c == NULL) {
		log_e("SHT40_Measure: Fail! handle->hi2c == NULL");
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_MeasureCommand[handle->repeatability]);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("SHT40_Measure: Fail! @ Write Measure Cmd Fail (Code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	delay_ms(SHT40_MeasureDelay[handle->repeatability]);
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, handle->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("SHT40_Measure: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	if (!SHT40_CheckCRC(handle->raw_data)) {
		log_e("SHT40_Measure: Fail! @ Check CRC Fail");
		return SHT40_STATUS_ERR_CRC;
	}
	float temperature = SHT40_CalcTemperature(handle->raw_data);
	float humidity = SHT40_CalcHumidity(handle->raw_data);
	if (humidity < 0) {
		log_w("SHT40_Measure: Warning! humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("SHT40_Measure: Warning! humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_i("SHT40_Measure: Success! temperature=%f, humidity=%f", temperature, humidity);
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_HeaterMeasure(SHT40_Handle_t *handle, SHT40_Heater_t heater) {
	return SHT40_STATUS_ERR_BUSY;
	if (handle == NULL) {
		log_e("SHT40_HeaterMeasure: Fail! handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hi2c == NULL) {
		log_e("SHT40_HeaterMeasure: Fail! handle->hi2c == NULL");
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	if (heater < 0 || heater > SHT40_HEATER_20MW100MS) {
		log_e("SHT40_HeaterMeasure: Fail! heater INVALID");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_HeaterMeasureCommand[heater]);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("SHT40_HeaterMeasure: Fail! @ Write Heater Measure Cmd Fail (Code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	delay_ms(SHT40_HeaterMeasureDelay[(heater % 2)]);
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, handle->raw_data, 6);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("SHT40_HeaterMeasure: Fail! @ Read Data Fail (Code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	if (!SHT40_CheckCRC(handle->raw_data)) {
		log_e("SHT40_HeaterMeasure: Fail! @ Check CRC Fail");
		return SHT40_STATUS_ERR_CRC;
	}
	float temperature = SHT40_CalcTemperature(handle->raw_data);
	float humidity = SHT40_CalcHumidity(handle->raw_data);
	if (humidity < 0) {
		log_w("SHT40_HeaterMeasure: Warning! humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("SHT40_HeaterMeasure: Warning! humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_i("SHT40_HeaterMeasure: Success! temperature=%f, humidity=%f", temperature, humidity);
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_SetRepeatability(SHT40_Handle_t *handle, SHT40_Repeatability_t rep) {
	if (handle == NULL) {
		log_e("SHT40_SetRepeatability: Fail! handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (rep < SHT40_REP_DEFAULT || rep > SHT40_REP_LOW) {
		log_w("SHT40_SetRepeatability: Warning! rep invalid! use SHT40_REP_DEFAULT");
		rep = SHT40_REP_DEFAULT;
	}
	handle->repeatability = rep;
	log_i("SHT40_SetRepeatability: Success! rep=%d", rep);
	return SHT40_STATUS_OK;
}
