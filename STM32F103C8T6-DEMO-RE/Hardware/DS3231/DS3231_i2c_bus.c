#include "DS3231.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "DS3231"
#include "elog.h"

DS3231_Status_t DS3231_Init(DS3231_Handle_t *dev, void *hi2c, uint8_t i2c_addr) {
	if (dev == NULL) {
		log_e("DS3231_Init: Fail! dev == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (hi2c == NULL) {
		log_e("DS3231_Init: Fail! hi2c == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	dev->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("DS3231_Init: Info Using default addr 0x%02X", DS3231_DEFAULT_I2C_ADDR);
		i2c_addr = DS3231_DEFAULT_I2C_ADDR;
	}
	dev->i2c_addr = i2c_addr;
	uint8_t status;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(dev->hi2c, dev->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("DS3231_Init: Fail! @ Read Status Register Fail (Code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("DS3231_Init: Success! status=0x%02X", status);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_Reset(DS3231_Handle_t *dev) {
	if (dev == NULL) {
		log_e("DS3231_Reset: Fail! dev == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("DS3231_Reset: Fail! dev->hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	uint8_t status;
	i2c_ret = i2c_read_reg(dev->hi2c, dev->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("DS3231_ReadStatus: Fail! @ Read Status Register Fail (Code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_i("DS3231_ReadStatus: Success! status=0x%02X");
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ReadStatus(DS3231_Handle_t *dev) {
	if (dev == NULL) {
		log_e("DS3231_ReadStatus: Fail! dev == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("DS3231_ReadStatus: Fail! dev->hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	uint8_t status;
	i2c_ret = i2c_read_reg(dev->hi2c, dev->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("DS3231_ReadStatus: Fail! @ Read Status Register Fail (Code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_i("DS3231_ReadStatus: Success! status=0x%02X", status);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ReadControl(DS3231_Handle_t *dev) {
	if (dev == NULL) {
		log_e("DS3231_ReadControl: Fail! dev == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("DS3231_ReadControl: Fail! dev->hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	uint8_t control;
	i2c_ret = i2c_read_reg(dev->hi2c, dev->i2c_addr, DS3231_REG_CONTROL, &control);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("DS3231_ReadControl: Fail! @ Read Control Register Fail (Code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_i("DS3231_ReadControl: Success! control=0x%02X", control);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ReadTime(DS3231_Handle_t *dev) {
	if (dev == NULL) {
		log_e("DS3231_ReadTime: Fail! dev == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (dev->hi2c == NULL) {
		log_e("DS3231_ReadTime: Fail! dev->hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_bus_status_t i2c_ret;
	uint8_t raw_time[7];
	i2c_ret = i2c_read_regs(dev->hi2c, dev->i2c_addr, DS3231_REG_SECONDS, raw_time, 7);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("DS3231_ReadTime: Fail! @ Read Seconds Register Fail (Code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	for (uint8_t i = 0; i < 7; i++) {
		log_i("DS3231_ReadTime: raw_time[%d]=0x%02X to-dec=%d", i, raw_time[i], DS3231_BCD2DEC(raw_time[i]));
	}
	return DS3231_STATUS_OK;
}
