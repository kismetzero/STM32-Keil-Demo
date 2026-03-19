#include "spi_bus.h"

#define LOG_TAG "spi_core"
#include "elog.h"

spi_bus_status_t spi_cs_low(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->low == NULL) {
		log_e("spi_cs_low: Fail handle/cs/ops/low == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	return handle->cs->ops->low(handle->cs->user_data);
}

spi_bus_status_t spi_cs_high(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->high == NULL) {
		log_e("spi_cs_high: Fail handle/cs/ops/high == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	return handle->cs->ops->high(handle->cs->user_data);
}

spi_bus_status_t spi_master_transmit(spi_dev_handle_t *handle, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL) {
		log_e("spi_master_transmit: Fail handle/bus/ops == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("spi_master_transmit: Fail data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_master_transmit: Fail len == 0");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_status_t ret;
	if (handle->bus->ops->write_bytes != NULL) {
		return handle->bus->ops->write_bytes(handle->bus->user_data, data, len);
	}
	if (handle->bus->ops->switch_bytes != NULL) {
		return handle->bus->ops->switch_bytes(handle->bus->user_data, data, NULL, len);
	}
	if (handle->bus->ops->write_byte != NULL) {
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->bus->ops->write_byte(handle->bus->user_data, data[i]);
			if (ret != SPI_BUS_OK) { return ret; }
		}
		return SPI_BUS_OK;
	}
	if (handle->bus->ops->switch_byte != NULL) {
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->bus->ops->switch_byte(handle->bus->user_data, data[i], NULL);
			if (ret != SPI_BUS_OK) { return ret; }
		}
		return SPI_BUS_OK;
	}
	log_e("spi_master_transmit: Fail handle->bus->ops->??? NONE");
	return SPI_BUS_ERR_INVALID_PARAM;
}

spi_bus_status_t spi_master_receive(spi_dev_handle_t *handle, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL) {
		log_e("spi_master_receive: Fail handle/bus/ops == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("spi_master_receive: Fail data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_master_receive: Fail len == 0");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_status_t ret;
	if (handle->bus->ops->read_bytes != NULL) {
		return handle->bus->ops->read_bytes(handle->bus->user_data, data, len);
	}
	if (handle->bus->ops->switch_bytes != NULL) {
		return handle->bus->ops->switch_bytes(handle->bus->user_data, NULL, data, len);
	}
	if (handle->bus->ops->read_byte != NULL) {
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->bus->ops->read_byte(handle->bus->user_data, &data[i]);
			if (ret != SPI_BUS_OK) { return ret; }
		}
		return SPI_BUS_OK;
	}
	if (handle->bus->ops->switch_byte != NULL) {
		for (uint16_t i = 0; i < len; i++) {
			ret = handle->bus->ops->switch_byte(handle->bus->user_data, NULL, &data[i]);
			if (ret != SPI_BUS_OK) { return ret; }
		}
		return SPI_BUS_OK;
	}
	log_e("spi_master_receive: Fail handle->bus->ops->??? NONE");
	return SPI_BUS_ERR_INVALID_PARAM;
}

spi_bus_status_t spi_bus_set_mode(spi_dev_handle_t *handle, spi_bus_mode_t mode) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL) {
		log_e("spi_bus_set_mode: Fail handle/bus/ops == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (handle->bus->ops->set_mode == NULL) {
		log_w("spi_bus_set_mode: Warning handle->bus->ops->set_mode == NULL");
		// 策略选择：
		// A. 返回错误 (严格模式) -> return SPI_BUS_ERR_NOT_SUPPORTED;
		// B. 返回成功但记录警告 (兼容模式，假设默认模式可用) -> 这里选择兼容模式
		return SPI_BUS_OK; 
	}
	if (mode > 3) {
		log_w("spi_bus_set_mode: Invalid mode %d requested", (int)mode);
		// 不直接拦截，传给底层驱动去处理（利用底层的自动修正逻辑）
	}
	spi_bus_status_t ret = handle->bus->ops->set_mode(handle->bus->user_data, mode);
	if (ret == SPI_BUS_OK) {
		log_i("spi_bus_set_mode: Success (Mode %d)", (int)mode);
	} else {
		log_e("spi_bus_set_mode: Failed with code %d", ret);
	}
	return ret;
}
