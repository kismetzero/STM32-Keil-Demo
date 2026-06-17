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
		log_e("handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited == 1) {
		log_d("already init");
		return SHT40_STATUS_OK;
	}
	if (hi2c == NULL) {
		log_e("hi2c == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	handle->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("using default i2c addr 0x%02X", SHT40_DEFAULT_I2C_ADDR);
		i2c_addr = SHT40_DEFAULT_I2C_ADDR;
	}
	handle->i2c_addr = i2c_addr;
	if (rep < SHT40_REP_DEFAULT || rep > SHT40_REP_LOW) {
		log_w("rep invalid! use default rep");
		rep = SHT40_REP_DEFAULT;
	}
	handle->repeatability = rep;
	handle->inited = 1;
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_Reset(SHT40_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return SHT40_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t i2c_ret;
#if I2C_BUS_SIMP <= 2 || !defined(I2C_BUS_SIMP)
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_ResetCommand);
#else /* I2C_BUS_SIMP */
	i2c_ret = i2c_master_trans(handle->hi2c, handle->i2c_addr, &SHT40_ResetCommand, 1, NULL, 0);
#endif /* I2C_BUS_SIMP */
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_Measure(SHT40_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return SHT40_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t i2c_ret;
#if I2C_BUS_SIMP <= 2 || !defined(I2C_BUS_SIMP)
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_MeasureCommand[handle->repeatability]);
#else /* I2C_BUS_SIMP */
	i2c_ret = i2c_master_trans(handle->hi2c, handle->i2c_addr, &SHT40_MeasureCommand[handle->repeatability], 1, NULL, 0);
#endif /* I2C_BUS_SIMP */
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	delay_ms(SHT40_MeasureDelay[handle->repeatability]);
	uint8_t raw_data[6];
#if I2C_BUS_SIMP <= 2 || !defined(I2C_BUS_SIMP)
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, raw_data, 6);
#else /* I2C_BUS_SIMP */
	i2c_ret = i2c_master_trans(handle->hi2c, handle->i2c_addr, NULL, 0, raw_data, 6);
#endif /* I2C_BUS_SIMP */
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	if (!SHT40_CheckCRC(raw_data)) {
		log_e("check crc fail");
		return SHT40_STATUS_ERR_CRC;
	}
	float temperature = SHT40_CalcTemperature(raw_data);
	float humidity = SHT40_CalcHumidity(raw_data);
	if (humidity < 0) {
		log_w("humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_d("temperature=%f, humidity=%f", temperature, humidity);
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_HeaterMeasure(SHT40_Handle_t *handle, SHT40_Heater_t heater) {
	return SHT40_STATUS_ERR_BUSY;
	if (handle == NULL) {
		log_e("handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return SHT40_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (heater < SHT40_HEATER_200MW1S || heater > SHT40_HEATER_20MW100MS) {
		log_e("heater invalid");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t i2c_ret;
#if I2C_BUS_SIMP <= 2 || !defined(I2C_BUS_SIMP)
	i2c_ret = i2c_write_byte(handle->hi2c, handle->i2c_addr, SHT40_HeaterMeasureCommand[heater]);
#else /* I2C_BUS_SIMP */
	i2c_ret = i2c_master_trans(handle->hi2c, handle->i2c_addr, &SHT40_HeaterMeasureCommand[heater], 1, NULL, 0);
#endif /* I2C_BUS_SIMP */
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	delay_ms(SHT40_HeaterMeasureDelay[(heater % 2)]);
	uint8_t raw_data[6];
#if I2C_BUS_SIMP <= 2 || !defined(I2C_BUS_SIMP)
	i2c_ret = i2c_read_bytes(handle->hi2c, handle->i2c_addr, raw_data, 6);
#else /* I2C_BUS_SIMP */
	i2c_ret = i2c_master_trans(handle->hi2c, handle->i2c_addr, NULL, 0, raw_data, 6);
#endif /* I2C_BUS_SIMP */
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return SHT40_STATUS_ERR_I2C_ERR;
	}
	if (!SHT40_CheckCRC(raw_data)) {
		log_e("check crc fail");
		return SHT40_STATUS_ERR_CRC;
	}
	float temperature = SHT40_CalcTemperature(raw_data);
	float humidity = SHT40_CalcHumidity(raw_data);
	if (humidity < 0) {
		log_w("humidity < 0");
		humidity = 0;
	} else if (humidity > 100) {
		log_w("humidity > 100");
		humidity = 100;
	}
	handle->temperature = temperature;
	handle->humidity = humidity;
	log_i("temperature=%f, humidity=%f", temperature, humidity);
	return SHT40_STATUS_OK;
}

SHT40_Status_t SHT40_SetRepeatability(SHT40_Handle_t *handle, SHT40_Repeatability_t rep) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return SHT40_STATUS_ERR_INVALID_PARAM;
	}
	if (rep < SHT40_REP_DEFAULT || rep > SHT40_REP_LOW) {
		log_w("rep invalid! use default rep");
		rep = SHT40_REP_DEFAULT;
	}
	handle->repeatability = rep;
	return SHT40_STATUS_OK;
}
