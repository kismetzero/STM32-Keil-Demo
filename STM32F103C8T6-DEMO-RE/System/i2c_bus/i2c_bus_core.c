#include "i2c_bus.h"
#include <stddef.h>

i2c_bus_status_t i2c_master_transmit(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
    if (handle == NULL || handle->ops == NULL || data == NULL || len == 0)
	{ return I2C_BUS_ERR_INVALID_PARAM; }
	
    if (handle->ops->start != NULL && handle->ops->write_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		// 1. 发送起始信号
		ret = handle->ops->start(handle->user_data);
		if (ret != I2C_BUS_OK) { return ret; }
		// 2. 发送设备地址 (写位 0)
		ret = handle->ops->write_byte(handle->user_data, (dev_addr << 1) | 0x00);
		if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		// 3. 发送数据
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->ops->write_byte(handle->user_data, data[i]);
			if (ret != I2C_BUS_OK) { handle->ops->stop(handle->user_data); return ret; }
		}
		// 4. 发送停止信号
		return handle->ops->stop(handle->user_data);
	}
	
	if (handle->ops->write_bytes != NULL) {
		return handle->ops->write_bytes(handle->user_data, dev_addr, data, len);
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
		ret = handle->ops->write_byte(handle->user_data, (dev_addr << 1) | 0x01);
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
	
	if (handle->ops->read_bytes != NULL) {
		return handle->ops->read_bytes(handle->user_data, dev_addr, data, len);
	}
	
	return I2C_BUS_ERR_INVALID_PARAM;
}