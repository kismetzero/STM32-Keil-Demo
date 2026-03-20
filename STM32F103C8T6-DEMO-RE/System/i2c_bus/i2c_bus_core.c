#include "i2c_bus.h"

#define LOG_TAG "i2c_core"
#include "elog.h"

i2c_bus_status_t i2c_start(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->start == NULL) {
		log_e("i2c_start: Fail handle/ops/start == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->start(handle);
}

i2c_bus_status_t i2c_stop(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->stop == NULL) {
		log_e("i2c_stop: Fail handle/ops/stop == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->stop(handle);
}

i2c_bus_status_t i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack) {
	if (handle == NULL || handle->ops == NULL || handle->ops->recv_byte == NULL) {
		log_e("i2c_recv_byte: Fail handle/ops/read_byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (byte == NULL) {
		log_e("i2c_recv_byte: Fail byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->recv_byte(handle, byte, ack);
}

i2c_bus_status_t i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte, bool wait) {
	if (handle == NULL || handle->ops == NULL || handle->ops->send_byte == NULL) {
		log_e("i2c_send_byte: Fail handle/ops/send_byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->send_byte(handle, byte, wait);
}

i2c_bus_status_t i2c_read_data(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL) {
		log_e("i2c_read_data: Fail handle/ops == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("i2c_read_data: Fail data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("i2c_read_data: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	if (handle->ops->start != NULL && handle->ops->send_byte != NULL
		&& handle->ops->recv_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		
		// 发送起始信号
		ret = handle->ops->start(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			log_e("i2c_read_data: Fail @ START (Code: %d). Bus busy?", ret);
			return ret;
		}
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		ret = handle->ops->send_byte(handle, (dev_addr << 1) | 0x01, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_read_data: Fail @ dev_addr(R)=0x%02X NACK. Device not found?", dev_addr);
			return I2C_BUS_STATUS_ERR_DEV_NONE;
		}
		// 读取数据
		for (uint16_t i = 0; i < len; i++) {
			bool ack = (i < len - 1); // 最后一个字节发送 NACK
			ret = handle->ops->recv_byte(handle, &data[i], ack);
			if (ret != I2C_BUS_STATUS_OK) {
				handle->ops->stop(handle);
				log_e("i2c_read_data: Fail @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
				return ret;
			}
		}
		// 发送停止信号
		ret = handle->ops->stop(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			// Stop 失败比较少见，但如果失败意味着总线可能一直占着
			log_e("i2c_read_data: Fail @ STOP (Code: %d). Bus may be stuck!", ret);
			return ret;
		}

		log_d("i2c_read_data: Success");
		return I2C_BUS_STATUS_OK;
	}

	log_e("i2c_read_data: Fail handle->ops->??? NONE");
	return I2C_BUS_STATUS_ERR_INVALID_PARAM;
}

i2c_bus_status_t i2c_write_data(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL) {
		log_e("i2c_write_data: Fail handle/ops == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("i2c_write_data: Fail data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("i2c_write_data: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	if (handle->ops->start != NULL && handle->ops->send_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		
		// 发送起始信号
		ret = handle->ops->start(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			log_e("i2c_write_data: Fail @ START (Code: %d). Bus busy?", ret);
			return ret;
		}
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		ret = handle->ops->send_byte(handle, (dev_addr << 1) | 0x00, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_write_data: Fail @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
			return I2C_BUS_STATUS_ERR_DEV_NONE;
		}
		// 发送数据
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->ops->send_byte(handle, data[i], true);
			if (ret != I2C_BUS_STATUS_OK) {
				handle->ops->stop(handle);
				log_e("i2c_write_data: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
				return ret;
			}
		}
		// 发送停止信号
		ret = handle->ops->stop(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			// Stop 失败比较少见，但如果失败意味着总线可能一直占着
			log_e("i2c_write_data: Fail @ STOP (Code: %d). Bus may be stuck!", ret);
			return ret;
		}
		
		log_d("i2c_write_data: Success (Total %d bytes)", len);
		return I2C_BUS_STATUS_OK;
	}
	
	log_e("i2c_write_data: Fail handle->ops->??? NONE");
	return I2C_BUS_STATUS_ERR_INVALID_PARAM;
}

i2c_bus_status_t i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte) {
	return i2c_read_regs(handle, dev_addr, reg_addr, byte, 1);
}

i2c_bus_status_t i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
	return i2c_write_regs(handle, dev_addr, reg_addr, &byte, 1);
}

i2c_bus_status_t i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL) {
		log_e("i2c_read_regs: Fail handle/ops == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("i2c_read_regs: Fail data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("i2c_read_regs: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	if (handle->ops->start != NULL && handle->ops->send_byte != NULL
		&& handle->ops->recv_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		// 发送起始信号
		ret = handle->ops->start(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			log_e("i2c_read_regs: Fail @ START (Code: %d). Bus busy?", ret);
			return ret;
		}
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		ret = handle->ops->send_byte(handle, (dev_addr << 1) | 0x00, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_read_regs: Fail @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
			return I2C_BUS_STATUS_ERR_DEV_NONE;
		}
		// 发送寄存器地址
		ret = handle->ops->send_byte(handle, reg_addr, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_read_regs: Fail @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK", dev_addr, reg_addr);
			return ret;
		}
		// 重复起始信号
		ret = handle->ops->start(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			// 此时总线状态不确定，尝试发个 Stop 保护一下
			handle->ops->stop(handle); 
			log_e("i2c_read_regs: Fail @ ReStart failed (Code: %d)", ret);
			return ret;
		}
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		ret = handle->ops->send_byte(handle, (dev_addr << 1) | 0x01, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_read_regs: Fail @ dev_addr(R)=0x%02X NACK. Device not found?", dev_addr);
			return I2C_BUS_STATUS_ERR_DEV_NONE;
		}
		// 读取数据
		for (uint16_t i = 0; i < len; i++) {
			bool ack = (i < len - 1); // 最后一个字节发送 NACK
			ret = handle->ops->recv_byte(handle, &data[i], ack);
			if (ret != I2C_BUS_STATUS_OK) {
				log_e("i2c_read_regs: Fail @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
				handle->ops->stop(handle);
				return ret;
			}
		}
		// 发送停止信号
		ret = handle->ops->stop(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			// Stop 失败比较少见，但如果失败意味着总线可能一直占着
			log_e("i2c_read_regs: Fail @ STOP (Code: %d). Bus may be stuck!", ret);
			return ret;
		}
		
		log_d("i2c_read_data: Success");
		return I2C_BUS_STATUS_OK;
	}
	
	log_e("i2c_read_regs: Fail handle->ops->??? NONE");
	return I2C_BUS_STATUS_ERR_INVALID_PARAM;
}

i2c_bus_status_t i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL) {
		log_e("i2c_write_regs: Fail handle/ops == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("i2c_write_regs: Fail data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("i2c_write_regs: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	if (handle->ops->start != NULL && handle->ops->send_byte != NULL && handle->ops->stop != NULL) {
		i2c_bus_status_t ret;
		
		// 发送起始信号
		ret = handle->ops->start(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			log_e("i2c_write_regs: Fail @ START (Code: %d). Bus busy?", ret);
			return ret;
		}
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		ret = handle->ops->send_byte(handle, (dev_addr << 1) | 0x00, true);
		if (ret != I2C_BUS_STATUS_OK) {
			handle->ops->stop(handle);
			log_e("i2c_write_regs: Fail @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
			return I2C_BUS_STATUS_ERR_DEV_NONE;
		}
		// 发送寄存器地址
		ret = handle->ops->send_byte(handle, reg_addr, true);
		if (ret != I2C_BUS_STATUS_OK) {
			log_e("i2c_write_regs: Fail @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK", dev_addr, reg_addr);
			handle->ops->stop(handle);
			return ret;
		}
		// 发送数据
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->ops->send_byte(handle, data[i], true);
			if (ret != I2C_BUS_STATUS_OK) {
				log_e("i2c_write_regs: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
				handle->ops->stop(handle);
				return ret;
			}
		}
		// 发送停止信号
		ret = handle->ops->stop(handle);
		if (ret != I2C_BUS_STATUS_OK) {
			// Stop 失败比较少见，但如果失败意味着总线可能一直占着
			log_e("i2c_write_regs: Fail @ STOP (Code: %d). Bus may be stuck!", ret);
			return ret;
		}
		
		log_d("i2c_write_regs: Success");
		return I2C_BUS_STATUS_OK;
	}
	
	log_e("i2c_write_regs: Fail handle->ops->??? NONE");
	return I2C_BUS_STATUS_ERR_INVALID_PARAM;
}
