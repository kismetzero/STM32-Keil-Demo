#include "spi_bus.h"
#include <stddef.h>

spi_bus_status_t spi_cs_low(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->low == NULL) {
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	return handle->cs->ops->low(handle->cs->user_data);
}

spi_bus_status_t spi_cs_high(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->high == NULL) {
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	return handle->cs->ops->high(handle->cs->user_data);
}

spi_bus_status_t spi_master_transmit(spi_dev_handle_t *handle, const uint8_t *data, uint16_t len) {
	if (handle == NULL || data == NULL || len == 0) { return SPI_BUS_ERR_INVALID_PARAM; }
	if (handle->bus == NULL || handle->bus->ops == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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
	
	return SPI_BUS_ERR_INVALID_PARAM;
}

spi_bus_status_t spi_master_receive(spi_dev_handle_t *handle, uint8_t *data, uint16_t len) {
    if (handle == NULL || data == NULL || len == 0) { return SPI_BUS_ERR_INVALID_PARAM; }
	if (handle->bus == NULL || handle->bus->ops == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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
	
	return SPI_BUS_ERR_INVALID_PARAM;
}