#include "DS3231.h"
#include "i2c_bus.h"
#include "delay.h"

#define LOG_TAG "DS3231"
#include "elog.h"

DS3231_Status_t DS3231_Init(DS3231_Handle_t *handle, void *hi2c, uint8_t i2c_addr) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited == 1) {
		log_d("already init");
		return DS3231_STATUS_OK;
	}
	if (hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	handle->hi2c = hi2c;
	if (i2c_addr == 0) {
		log_i("using default i2c addr 0x%02X", DS3231_DEFAULT_I2C_ADDR);
		i2c_addr = DS3231_DEFAULT_I2C_ADDR;
	}
	handle->i2c_addr = i2c_addr;
	uint8_t status = 0;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	
	if (status & DS3231_BIT_STATUS_OSF) {
		log_w("Oscillator stopped (OSF=1). Time needs to be set.");
	}
	
	status &= ~(DS3231_BIT_STATUS_A1F | DS3231_BIT_STATUS_A2F);
	
	i2c_ret = i2c_write_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	handle->inited = 1;
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_SoftwareReset(DS3231_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t status = 0;
	uint8_t control = 0;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_CONTROL, &control);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	
	if (status & DS3231_BIT_STATUS_OSF) {
		log_w("Oscillator stopped (OSF=1). Time needs to be set.");
	}
	
	status &= ~(DS3231_BIT_STATUS_A1F | DS3231_BIT_STATUS_A2F | DS3231_BIT_STATUS_EN32KHZ);
	control = DS3231_BIT_CTRL_INTCN;
	
	i2c_ret = i2c_write_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	i2c_ret = i2c_write_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_CONTROL, control);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ClearOSF(DS3231_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t status = 0;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	
	status &= ~DS3231_BIT_STATUS_OSF;
	
	i2c_ret = i2c_write_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ReadControlRegister(DS3231_Handle_t *handle, uint8_t *data) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t control = 0;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_CONTROL, &control);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("control=0x%02X EOSC=%d BBSQW=%d CONV=%d RS2=%d RS1=%d INTCN=%d A2IE=%d A1IE=%d", control,
								(control & DS3231_BIT_CTRL_EOSC) != 0,
								(control & DS3231_BIT_CTRL_BBSQW) != 0,
								(control & DS3231_BIT_CTRL_CONV) != 0,
								(control & DS3231_BIT_CTRL_RS2) != 0,
								(control & DS3231_BIT_CTRL_RS1) != 0,
								(control & DS3231_BIT_CTRL_INTCN) != 0,
								(control & DS3231_BIT_CTRL_A2IE) != 0,
								(control & DS3231_BIT_CTRL_A1IE) != 0);
	if (data != NULL) { *data = control; }
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_ReadStatusRegister(DS3231_Handle_t *handle, uint8_t *data) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t status = 0;
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_reg(handle->hi2c, handle->i2c_addr, DS3231_REG_STATUS, &status);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("status=0x%02X OSF=%d EN32KHZ=%d BSY=%d A2F=%d A1F=%d", status,
								(status & DS3231_BIT_STATUS_OSF) != 0,
								(status & DS3231_BIT_STATUS_EN32KHZ) != 0,
								(status & DS3231_BIT_STATUS_BSY) != 0,
								(status & DS3231_BIT_STATUS_A2F) != 0,
								(status & DS3231_BIT_STATUS_A1F) != 0);
	if (data != NULL) { *data = status; }
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_GetDateTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[7];
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_SEC, raw_data, 7);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	dt->sec = DS3231_BCD2DEC(raw_data[0] & DS3231_MASK_SEC);
	dt->min = DS3231_BCD2DEC(raw_data[1] & DS3231_MASK_MIN);
	if (raw_data[2] & DS3231_BIT_HOUR_EN12H) {
		dt->en12h = true;
		dt->hour = DS3231_BCD2DEC(raw_data[2] & DS3231_MASK_HOUR_12H);
		dt->pm = (raw_data[2] & DS3231_BIT_HOUR_PM) ? true : false;
	} else {
		dt->en12h = false;
		dt->hour = DS3231_BCD2DEC(raw_data[2] & DS3231_MASK_HOUR_24H);
	}
	dt->week = DS3231_BCD2DEC(raw_data[3] & DS3231_MASK_DAY);
	dt->date = DS3231_BCD2DEC(raw_data[4] & DS3231_MASK_DATE);
	dt->month = DS3231_BCD2DEC(raw_data[5] & DS3231_MASK_MONTH);
	dt->year = DS3231_BCD2DEC(raw_data[6] & DS3231_MASK_YEAR) + ((raw_data[5] & DS3231_BIT_MONTH_CENTURY) ? 2100 : 2000);
	log_d("%02d:%02d:%02d %04d-%02d-%02d week=%d en12h=%d pm=%d",
							dt->hour, dt->min, dt->sec,
							dt->year, dt->month ,dt->date,
							dt->week, dt->en12h, dt->pm);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_SetDateTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[7];
	raw_data[0] = DS3231_DEC2BCD(dt->sec & DS3231_MASK_SEC);
	raw_data[1] = DS3231_DEC2BCD(dt->min & DS3231_MASK_MIN);
	uint8_t raw_hour = dt->hour;
	if (dt->en12h) {
		raw_hour &= DS3231_MASK_HOUR_12H;
		raw_hour |= DS3231_BIT_HOUR_EN12H;
		if (dt->pm) { raw_hour |= DS3231_BIT_HOUR_PM; }
		else { raw_hour &= ~DS3231_BIT_HOUR_PM;}
	} else {
		raw_hour &= DS3231_MASK_HOUR_24H;
		raw_hour &= ~DS3231_BIT_HOUR_EN12H;
	}
	raw_data[2] = DS3231_DEC2BCD(raw_hour);
	raw_data[3] = DS3231_DEC2BCD(dt->week & DS3231_MASK_DAY);
	raw_data[4] = DS3231_DEC2BCD(dt->date & DS3231_MASK_DATE);
	uint8_t raw_month = dt->month & DS3231_MASK_MONTH;
	if (dt->year / 100 == 21) { raw_month |= DS3231_BIT_MONTH_CENTURY; }
	raw_data[5] = DS3231_DEC2BCD(raw_month);
	raw_data[6] = DS3231_DEC2BCD((dt->year % 100) & DS3231_MASK_YEAR);
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_SEC, raw_data, 7);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("%02d:%02d:%02d %04d-%02d-%02d week=%d en12h=%d pm=%d",
							dt->hour, dt->min, dt->sec,
							dt->year, dt->month ,dt->date,
							dt->week, dt->en12h, dt->pm);
	return DS3231_ClearOSF(handle);
}

DS3231_Status_t DS3231_GetTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[3];
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_SEC, raw_data, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	dt->sec = DS3231_BCD2DEC(raw_data[0] & DS3231_MASK_SEC);
	dt->min = DS3231_BCD2DEC(raw_data[1] & DS3231_MASK_MIN);
	if (raw_data[2] & DS3231_BIT_HOUR_EN12H) {
		dt->en12h = true;
		dt->hour = DS3231_BCD2DEC(raw_data[2] & DS3231_MASK_HOUR_12H);
		dt->pm = (raw_data[2] & DS3231_BIT_HOUR_PM) ? true : false;
	} else {
		dt->en12h = false;
		dt->hour = DS3231_BCD2DEC(raw_data[2] & DS3231_MASK_HOUR_24H);
	}
	log_d("%02d:%02d:%02d", dt->hour, dt->min, dt->sec);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_SetTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[3];
	raw_data[0] = DS3231_DEC2BCD(dt->sec & DS3231_MASK_SEC);
	raw_data[1] = DS3231_DEC2BCD(dt->min & DS3231_MASK_MIN);
	uint8_t raw_hour = dt->hour;
	if (dt->en12h) {
		raw_hour &= DS3231_MASK_HOUR_12H;
		raw_hour |= DS3231_BIT_HOUR_EN12H;
		if (dt->pm) { raw_hour |= DS3231_BIT_HOUR_PM; }
		else { raw_hour &= ~DS3231_BIT_HOUR_PM;}
	} else {
		raw_hour &= DS3231_MASK_HOUR_24H;
		raw_hour &= ~DS3231_BIT_HOUR_EN12H;
	}
	raw_data[2] = DS3231_DEC2BCD(raw_hour);
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_SEC, raw_data, 3);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("%02d:%02d:%02d", dt->hour, dt->min, dt->sec);
	return DS3231_ClearOSF(handle);
}

DS3231_Status_t DS3231_GetDate(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[4];
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_read_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_DAY, raw_data, 4);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c read fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	dt->week = DS3231_BCD2DEC(raw_data[0] & DS3231_MASK_DAY);
	dt->date = DS3231_BCD2DEC(raw_data[1] & DS3231_MASK_DATE);
	dt->month = DS3231_BCD2DEC(raw_data[2] & DS3231_MASK_MONTH);
	dt->year = DS3231_BCD2DEC(raw_data[3] & DS3231_MASK_YEAR) + ((raw_data[2] & DS3231_BIT_MONTH_CENTURY) ? 2100 : 2000);
	log_d("%04d-%02d-%02d week=%d en12h=%d pm=%d",
							dt->year, dt->month ,dt->date,
							dt->week, dt->en12h, dt->pm);
	return DS3231_STATUS_OK;
}

DS3231_Status_t DS3231_SetDate(DS3231_Handle_t *handle, DS3231_DateTime_t *dt) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return DS3231_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return DS3231_STATUS_ERR_NO_INIT;
	}
	if (handle->hi2c == NULL) {
		log_e("hi2c == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	if (dt == NULL) {
		log_e("dt == NULL");
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	uint8_t raw_data[4];
	raw_data[0] = DS3231_DEC2BCD(dt->week & DS3231_MASK_DAY);
	raw_data[1] = DS3231_DEC2BCD(dt->date & DS3231_MASK_DATE);
	uint8_t raw_month = dt->month & DS3231_MASK_MONTH;
	if (dt->year / 100 == 21) { raw_month |= DS3231_BIT_MONTH_CENTURY; }
	raw_data[2] = DS3231_DEC2BCD(raw_month);
	raw_data[3] = DS3231_DEC2BCD((dt->year % 100) & DS3231_MASK_YEAR);
	i2c_bus_status_t i2c_ret;
	i2c_ret = i2c_write_regs(handle->hi2c, handle->i2c_addr, DS3231_REG_DAY, raw_data, 4);
	if (i2c_ret != I2C_BUS_STATUS_OK) {
		log_e("i2c write fail (code: %d)", i2c_ret);
		return DS3231_STATUS_ERR_I2C_ERR;
	}
	log_d("%04d-%02d-%02d week=%d en12h=%d pm=%d",
							dt->year, dt->month ,dt->date,
							dt->week, dt->en12h, dt->pm);
	return DS3231_ClearOSF(handle);
}
