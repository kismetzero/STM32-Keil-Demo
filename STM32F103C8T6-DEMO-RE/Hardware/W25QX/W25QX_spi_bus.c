#include "W25QX.h"
#include "spi_bus.h"

#define LOG_TAG "W25QX"
#include "elog.h"

W25QX_Status_t W25QX_Init(W25QX_Handle_t *handle, void *hspi) {
	if (handle == NULL) {
		log_e("W25QX_Init: Fail! handle == NULL");
		return W25QX_STATUS_ERR_INVALID_PARAM;
	}
	if (hspi == NULL) {
		log_e("W25QX_Init: Fail! hspi == NULL");
		return W25QX_STATUS_ERR_INVALID_PARAM;
	}
	handle->hspi = hspi;
	W25QX_Status_t ret = W25QX_ReadID(handle);
	if (ret != W25QX_STATUS_OK) {
		log_e("W25QX_Init: Fail! Read ID Fail! (Code: %d)", ret);
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	log_i("W25QX_Init: Success!");
	return W25QX_STATUS_OK;
}

W25QX_Status_t W25QX_Reset(W25QX_Handle_t *handle) {
	if (handle == NULL) {
		log_e("W25QX_Reset: Fail! handle == NULL");
		return W25QX_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hspi == NULL) {
		log_e("W25QX_Reset: Fail! handle->hspi == NULL");
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	return W25QX_STATUS_OK;
}

W25QX_Status_t W25QX_ReadID(W25QX_Handle_t *handle) {
	if (handle == NULL) {
		log_e("W25QX_ReadID: Fail! handle == NULL");
		return W25QX_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->hspi == NULL) {
		log_e("W25QX_ReadID: Fail! handle->hspi == NULL");
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	uint8_t raw_data[3] = {0};
	spi_bus_status_t ret;
	ret = spi_cs_low(handle->hspi);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("W25QX_ReadID: Fail! spi_cs_low fail (Code: %d)", ret);
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	ret = spi_write_byte(handle->hspi, 0x9F);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("W25QX_ReadID: Fail! spi_write_byte (Code: %d)", ret);
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	ret = spi_read_bytes(handle->hspi, raw_data, 3);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("W25QX_ReadID: Fail! spi_read_bytes (Code: %d)", ret);
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	ret = spi_cs_high(handle->hspi);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("W25QX_ReadID: Fail! spi_cs_high fail (Code: %d)", ret);
		return W25QX_STATUS_ERR_SPI_ERR;
	}
	uint16_t did = (raw_data[1] << 8) | raw_data[2];
	log_i("W25QX_ReadID: Success! MID=0x%02X DID=0x%04X", raw_data[0], did);
	return W25QX_STATUS_OK;
}

