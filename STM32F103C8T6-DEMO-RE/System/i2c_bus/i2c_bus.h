#ifndef __I2C_BUS_H
#define __I2C_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	I2C_BUS_STATUS_OK = 0,
	I2C_BUS_STATUS_ERR,
	I2C_BUS_STATUS_ERR_INVALID_PARAM,
	I2C_BUS_STATUS_ERR_NO_INIT,
	I2C_BUS_STATUS_ERR_BUSY,
	I2C_BUS_STATUS_ERR_NACK,
	I2C_BUS_STATUS_ERR_TIMEOUT,
	I2C_BUS_STATUS_ERR_DEV_NONE,
	I2C_BUS_STATUS_ERR_REG_NONE,
} i2c_bus_status_t;

typedef struct i2c_bus_handle_s i2c_bus_handle_t;
typedef struct i2c_bus_ops_s i2c_bus_ops_t;

struct i2c_bus_handle_s {
	void *user_data; 
	const i2c_bus_ops_t *ops;
};

struct i2c_bus_ops_s {
	i2c_bus_status_t (*init)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*deinit)(i2c_bus_handle_t *handle);
	
	i2c_bus_status_t (*start)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*stop)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*send_ack)(i2c_bus_handle_t *handle, bool ack);
	i2c_bus_status_t (*wait_ack)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*recv_byte)(i2c_bus_handle_t *handle, uint8_t *byte);
	i2c_bus_status_t (*send_byte)(i2c_bus_handle_t *handle, uint8_t byte);
	
	i2c_bus_status_t (*read_byte)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte);
	i2c_bus_status_t (*write_byte)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte);
	i2c_bus_status_t (*read_bytes)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len);
	i2c_bus_status_t (*write_bytes)(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len);

	i2c_bus_status_t (*read_reg)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte);
	i2c_bus_status_t (*write_reg)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);
	i2c_bus_status_t (*read_regs)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
	i2c_bus_status_t (*write_regs)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
};

static inline i2c_bus_status_t i2c_init(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->init == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->init(handle);
}

static inline i2c_bus_status_t i2c_deinit(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->deinit == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->deinit(handle);
}

static inline i2c_bus_status_t i2c_start(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->start == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->start(handle);
}

static inline i2c_bus_status_t i2c_stop(i2c_bus_handle_t *handle) {
	if (handle == NULL || handle->ops == NULL || handle->ops->stop == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->stop(handle);
}

static inline i2c_bus_status_t i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->recv_byte == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->recv_byte(handle, byte);
}

static inline i2c_bus_status_t i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->send_byte == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->send_byte(handle, byte);
}

static inline i2c_bus_status_t i2c_read_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->read_byte == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->read_byte(handle, dev_addr, byte);
}

static inline i2c_bus_status_t i2c_write_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->write_byte == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->write_byte(handle, dev_addr, byte);
}

static inline i2c_bus_status_t i2c_read_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL || handle->ops->read_bytes == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->read_bytes(handle, dev_addr, data, len);
}

static inline i2c_bus_status_t i2c_write_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL || handle->ops->write_bytes == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->write_bytes(handle, dev_addr, data, len);
}

static inline i2c_bus_status_t i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->read_reg == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->read_reg(handle, dev_addr, reg_addr, byte);
}

static inline i2c_bus_status_t i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
	if (handle == NULL || handle->ops == NULL || handle->ops->write_reg == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->write_reg(handle, dev_addr, reg_addr, byte);
}

static inline i2c_bus_status_t i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL || handle->ops->read_regs == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->read_regs(handle, dev_addr, reg_addr, data, len);
}

static inline i2c_bus_status_t i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL || handle->ops == NULL || handle->ops->write_regs == NULL) {
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return handle->ops->write_regs(handle, dev_addr, reg_addr, data, len);
}

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_BUS_H */