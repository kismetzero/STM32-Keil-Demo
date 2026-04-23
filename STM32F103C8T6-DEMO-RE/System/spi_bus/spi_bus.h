#ifndef __SPI_BUS_H
#define __SPI_BUS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	SPI_BUS_STATUS_OK = 0,
	SPI_BUS_STATUS_ERR,
	SPI_BUS_STATUS_ERR_INVALID_PARAM,
	SPI_BUS_STATUS_ERR_NO_INIT,
	SPI_BUS_STATUS_ERR_BUSY,
	SPI_BUS_STATUS_ERR_TIMEOUT,
} spi_bus_status_t;

// SPI 模式定义 (CPOL, CPHA)
typedef enum {
	SPI_BUS_MODE_0	= 0, // CPOL=0, CPHA=0
	SPI_BUS_MODE_1	= 1, // CPOL=0, CPHA=1
	SPI_BUS_MODE_2	= 2, // CPOL=1, CPHA=0
	SPI_BUS_MODE_3	= 3  // CPOL=1, CPHA=1
} spi_bus_mode_t;

typedef struct spi_bus_handle_s spi_bus_handle_t;
typedef struct spi_bus_ops_s spi_bus_ops_t;

typedef struct spi_cs_handle_s spi_cs_handle_t;
typedef struct spi_cs_ops_s spi_cs_ops_t;

typedef struct spi_dev_handle_s spi_dev_handle_t;

struct spi_dev_handle_s {
	spi_cs_handle_t *cs;		// 关联的片选控制器
	spi_bus_handle_t *bus;		// 关联的总线
};

struct spi_cs_handle_s {
	void *user_data; 
	const spi_cs_ops_t *ops;
};

struct spi_cs_ops_s {
	spi_bus_status_t (*init)(spi_cs_handle_t *handle);
	spi_bus_status_t (*high)(spi_cs_handle_t *handle);
	spi_bus_status_t (*low)(spi_cs_handle_t *handle);
};

struct spi_bus_handle_s {
	void *user_data; 
	const spi_bus_ops_t *ops;
};

struct spi_bus_ops_s {
	spi_bus_status_t (*init)(spi_bus_handle_t *handle);
	spi_bus_status_t (*deinit)(spi_bus_handle_t *handle);
	spi_bus_status_t (*read_byte)(spi_bus_handle_t *handle, uint8_t *byte);
	spi_bus_status_t (*write_byte)(spi_bus_handle_t *handle, uint8_t byte);
	spi_bus_status_t (*switch_byte)(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx);
	
	spi_bus_status_t (*read_bytes)(spi_bus_handle_t *handle, uint8_t *data, uint16_t len);
	spi_bus_status_t (*write_bytes)(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len);
	spi_bus_status_t (*switch_bytes)(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len);
	
	// 动态配置 (SPI 特性：不同设备可能需要不同速率和模式)
	spi_bus_status_t (*set_speed)(spi_bus_handle_t *handle, uint32_t speed_hz);
	spi_bus_status_t (*set_mode)(spi_bus_handle_t *handle, spi_bus_mode_t mode);
};

static inline spi_bus_status_t spi_cs_init(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->init == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->cs->ops->init(handle->cs);
}

static inline spi_bus_status_t spi_cs_high(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->high == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->cs->ops->high(handle->cs);
}

static inline spi_bus_status_t spi_cs_low(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->cs == NULL || handle->cs->ops == NULL || handle->cs->ops->low == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->cs->ops->low(handle->cs);
}

static inline spi_bus_status_t spi_bus_init(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->init == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->init(handle->bus);
}

static inline spi_bus_status_t spi_bus_deinit(spi_dev_handle_t *handle) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->deinit == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->deinit(handle->bus);
}

static inline spi_bus_status_t spi_read_byte(spi_dev_handle_t *handle, uint8_t *byte) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->read_byte == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->read_byte(handle->bus, byte);
}

static inline spi_bus_status_t spi_write_byte(spi_dev_handle_t *handle, uint8_t byte) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->write_byte == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->write_byte(handle->bus, byte);
}

static inline spi_bus_status_t spi_switch_byte(spi_dev_handle_t *handle, uint8_t tx, uint8_t *rx) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->switch_byte == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->switch_byte(handle->bus, tx, rx);
}

static inline spi_bus_status_t spi_read_bytes(spi_dev_handle_t *handle, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->read_bytes == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->read_bytes(handle->bus, data, len);
}

static inline spi_bus_status_t spi_write_bytes(spi_dev_handle_t *handle, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->write_bytes == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->write_bytes(handle->bus, data, len);
}

static inline spi_bus_status_t spi_switch_bytes(spi_dev_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->switch_bytes == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->switch_bytes(handle->bus, tx, rx, len);
}

static inline spi_bus_status_t spi_dev_set_mode(spi_dev_handle_t *handle, spi_bus_mode_t mode) {
	if (handle == NULL || handle->bus == NULL || handle->bus->ops == NULL || handle->bus->ops->set_mode == NULL) {
        return SPI_BUS_STATUS_ERR_INVALID_PARAM;
    }
    return handle->bus->ops->set_mode(handle->bus, mode);
}

#ifdef __cplusplus
}
#endif

#endif	/* __SPI_BUS_H */