#include "i2c_bus.h"

i2c_bus_status_t i2c_start(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->start == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	return handle->ops->start(handle->user_data);
}

i2c_bus_status_t i2c_stop(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->stop == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	return handle->ops->stop(handle->user_data);
}

i2c_bus_status_t i2c_write_byte(i2c_bus_handle_t *handle, uint8_t byte, bool wait) {
	if (handle == NULL || handle->ops == NULL || handle->ops->write_byte == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	return handle->ops->write_byte(handle->user_data, byte, wait);
}

i2c_bus_status_t i2c_read_byte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack) {
	if (handle == NULL || handle->ops == NULL || handle->ops->read_byte == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	return handle->ops->read_byte(handle->user_data, byte, ack);
}

i2c_bus_status_t i2c_master_transmit(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
    if (handle == NULL || handle->ops == NULL || data == NULL || len == 0)
	{ return I2C_BUS_ERR_INVALID_PARAM; }
	
    if (handle->ops->start != NULL && handle->ops->write_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		// 1. 发送起始信号
		ret = handle->ops->start(handle->user_data);
		if (ret != I2C_BUS_OK) { return ret; }
		// 2. 发送设备地址 (写位 0)
		ret = handle->ops->write_byte(handle->user_data, (dev_addr << 1) | 0x00, true);
		if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		// 3. 发送数据
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->ops->write_byte(handle->user_data, data[i], true);
			if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		}
		// 4. 发送停止信号
		return handle->ops->stop(handle->user_data);
	}
	return I2C_BUS_ERR_INVALID_PARAM;
}

i2c_bus_status_t i2c_master_receive(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
    if (handle == NULL || handle->ops == NULL || data == NULL || len == 0)
	{ return I2C_BUS_ERR_INVALID_PARAM; }
	
	if (handle->ops->start != NULL && handle->ops->write_byte != NULL
		&& handle->ops->read_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		// 1. 发送起始信号
		ret = handle->ops->start(handle->user_data);
		if (ret != I2C_BUS_OK) { return ret; }
		// 2. 发送设备地址 (读位 1)
		ret = handle->ops->write_byte(handle->user_data, (dev_addr << 1) | 0x01, true);
		if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		// 3. 读取数据
		for (uint16_t i = 0; i < len; i++) {
			bool ack = (i < len - 1); // 最后一个字节发送 NACK
			ret = handle->ops->read_byte(handle->user_data, &data[i], ack);
			if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		}
		// 4. 发送停止信号
		return handle->ops->stop(handle->user_data);
	}
	return I2C_BUS_ERR_INVALID_PARAM;
}
