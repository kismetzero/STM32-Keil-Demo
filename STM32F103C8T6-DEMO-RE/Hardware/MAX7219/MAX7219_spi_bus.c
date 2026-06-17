#include "MAX7219.h"
#include "spi_bus.h"
#include <string.h>

#define LOG_TAG "MAX7219"
#include "elog.h"

#define MAX7219_CMD_COUNT 5
static const uint8_t MAX7219_InitCmd[MAX7219_CMD_COUNT][2] = {
	{ 0x0C, 0x00 },		// 工作
	{ 0x09, 0x00 },		// BCD 译码模式 关
	{ 0x0B, 0x07 },		// 扫描界限
	{ 0x0A, MAX7219_DEFAULT_INTENSITY },		// 亮度
	{ 0x0F, 0x00 }		// TEST 测试模式 关
};

static spi_bus_status_t MAX7219_write_cmd(MAX7219_Handle_t *handle, uint8_t reg_addr, uint8_t byte) {
	spi_bus_status_t spi_ret;
	spi_ret = spi_bus_lock(handle->hspi);
	if (spi_ret != SPI_BUS_STATUS_OK) { goto exit; }
	spi_ret = spi_cs_low(handle->hspi);
	if (spi_ret != SPI_BUS_STATUS_OK) { goto unlock; }
	for (uint8_t i = 0; i < handle->count; i++) {
		spi_ret = spi_write_byte(handle->hspi, reg_addr);
		if (spi_ret != SPI_BUS_STATUS_OK) { goto high; }
		spi_ret = spi_write_byte(handle->hspi, byte);
		if (spi_ret != SPI_BUS_STATUS_OK) { goto high; }
	}
	high:
	spi_cs_high(handle->hspi);
	unlock:
	spi_bus_unlock(handle->hspi);
	exit:
	if (spi_ret != SPI_BUS_STATUS_OK) {
		log_e("reg(0x%02X) byte(0x%02X) write fail (code: %d)", reg_addr, byte, spi_ret);
	}
	return spi_ret;
}

static spi_bus_status_t MAX7219_write_data(MAX7219_Handle_t *handle, const uint8_t *data, uint16_t len) {
	uint16_t i = 0;
	spi_bus_status_t spi_ret;
	spi_ret = spi_bus_lock(handle->hspi);
	if (spi_ret != SPI_BUS_STATUS_OK) { goto exit; }
	spi_ret = spi_cs_low(handle->hspi);
	if (spi_ret != SPI_BUS_STATUS_OK) { goto unlock; }
	for (i = 0; i < len; i++) {
		spi_ret = spi_write_byte(handle->hspi, data[i]);
		if (spi_ret != SPI_BUS_STATUS_OK) { goto high; }
	}
	high:
	spi_cs_high(handle->hspi);
	unlock:
	spi_bus_unlock(handle->hspi);
	exit:
	if (spi_ret != SPI_BUS_STATUS_OK) {
		log_e("data[%d]=0x%02X write fail (code: %d)", i, data[i], spi_ret);
	}
	return spi_ret;
}

MAX7219_Status_t MAX7219_Init(MAX7219_Handle_t *handle, void *hspi, uint8_t count) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited == 1) {
		log_d("already init");
		return MAX7219_STATUS_OK;
	}
	if (hspi == NULL) {
		log_e("hspi == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (count == 0) {
		log_e("count == 0");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	handle->hspi = hspi;
	handle->count = count;
	handle->intensity = MAX7219_DEFAULT_INTENSITY;
	spi_bus_status_t spi_ret;
	for (uint8_t i = 0; i < MAX7219_CMD_COUNT; i++) {
		spi_ret = MAX7219_write_cmd(handle, MAX7219_InitCmd[i][0], MAX7219_InitCmd[i][1]);
		if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	}
	for (uint8_t i = 0; i < 8; i++) {
		spi_ret = MAX7219_write_cmd(handle, i + 1, 0x00);
		if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	}
	spi_ret = MAX7219_write_cmd(handle, MAX7219_REG_EN, 0x01);
	if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	#if SYS_EN_FREERTOS
		if (handle->data == NULL) {
			handle->data = (uint8_t *)pvPortMalloc(count * 8);
			if (handle->data == NULL) {
				// 内存不足！
				// 在 FreeRTOS 中，如果配置了 configUSE_MALLOC_FAILED_HOOK，
				// 这里可能会自动触发钩子函数
				log_e("pvPortMalloc fail");
				return MAX7219_STATUS_ERR;
			}
			memset(handle->data, 0, count * 8);
		}
		if (handle->mutex_lock == NULL) {
			handle->mutex_lock = xSemaphoreCreateRecursiveMutex();
			if (handle->mutex_lock == NULL) {
				log_e("mutex_lock create fail");
				return MAX7219_STATUS_ERR;
			}
		}
	#endif /* SYS_EN_FREERTOS */
	handle->inited = 1;
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_DeInit(MAX7219_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		return MAX7219_STATUS_OK;
	}
	#if SYS_EN_FREERTOS
		if (handle->data != NULL) {
			vPortFree(handle->data);
            handle->data = NULL;
		}
		if (handle->mutex_lock != NULL) {
			vSemaphoreDelete(handle->mutex_lock);
            handle->mutex_lock = NULL;
		}
	#endif /* SYS_EN_FREERTOS */
	handle->inited = 0;
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_Lock(MAX7219_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	#if SYS_EN_FREERTOS
		if (xSemaphoreTakeRecursive(handle->mutex_lock, pdMS_TO_TICKS(1000)) != pdTRUE) {
			log_w("mutex_lock timeout");
			return MAX7219_STATUS_ERR;
		}
	#endif /* SYS_EN_FREERTOS */
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_Unlock(MAX7219_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	#if SYS_EN_FREERTOS
		xSemaphoreGiveRecursive(handle->mutex_lock);
	#endif /* SYS_EN_FREERTOS */
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_WriteCmd(MAX7219_Handle_t *handle, uint8_t reg_addr, uint8_t byte) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	spi_bus_status_t spi_ret = MAX7219_write_cmd(handle, reg_addr, byte);
	if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_WriteData(MAX7219_Handle_t *handle, uint8_t *data, uint16_t len) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	spi_bus_status_t spi_ret = MAX7219_write_data(handle, data, len);
	if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_SetIntensity(MAX7219_Handle_t *handle, uint8_t intensity) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	if (intensity > 15) {
		log_e("intensity=%d invalid! use intensity=%d.", intensity, MAX7219_DEFAULT_INTENSITY);
		intensity = MAX7219_DEFAULT_INTENSITY;
	}
	spi_bus_status_t spi_ret = MAX7219_write_cmd(handle, MAX7219_REG_INTENSITY, intensity);
	if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_Clear(MAX7219_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	if (handle->data != NULL) { memset(handle->data, 0, handle->count * 8); }
	spi_bus_status_t spi_ret;
	for (uint8_t i = 0; i < 8; i++) {
		spi_ret = MAX7219_write_cmd(handle, i + 1, 0x00);
		if (spi_ret != SPI_BUS_STATUS_OK) { return MAX7219_STATUS_ERR_SPI_ERR; }
	}
	return MAX7219_STATUS_OK;
}

MAX7219_Status_t MAX7219_Refresh(MAX7219_Handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return MAX7219_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->inited != 1) {
		log_e("no init");
		return MAX7219_STATUS_ERR_NO_INIT;
	}
	if (handle->data == NULL) {
		return MAX7219_STATUS_ERR;
	}
	uint16_t digit_addr;
	uint8_t chip_idx;
	spi_bus_status_t spi_ret;
	for (digit_addr = 1; digit_addr <= 8; digit_addr++) {
		spi_ret = spi_bus_lock(handle->hspi);
		if (spi_ret != SPI_BUS_STATUS_OK) { goto exit; }
		spi_ret = spi_cs_low(handle->hspi);
		if (spi_ret != SPI_BUS_STATUS_OK) { goto unlock; }
		for (chip_idx = 0; chip_idx < handle->count; chip_idx++) {
			spi_ret = spi_write_byte(handle->hspi, digit_addr);
			if (spi_ret != SPI_BUS_STATUS_OK) { goto high; }
			uint16_t mem_idx = (chip_idx * 8) + (digit_addr - 1);
			spi_ret = spi_write_byte(handle->hspi, handle->data[mem_idx]);
			if (spi_ret != SPI_BUS_STATUS_OK) { goto high; }
		}
		high:
		spi_cs_high(handle->hspi);
		unlock:
		spi_bus_unlock(handle->hspi);
		exit:
		if (spi_ret != SPI_BUS_STATUS_OK) {
			log_e("data[%d][%d]=0x%02X write fail (code: %d)", chip_idx, digit_addr, (&handle->data)[chip_idx][digit_addr], spi_ret);
			return MAX7219_STATUS_ERR_SPI_ERR;
		}
	}
	return MAX7219_STATUS_OK;
}
