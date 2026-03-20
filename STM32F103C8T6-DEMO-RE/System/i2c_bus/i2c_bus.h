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
	I2C_BUS_STATUS_ERR_BUSY,
	I2C_BUS_STATUS_ERR_DEV_NONE,
	I2C_BUS_STATUS_ERR_NACK,
	I2C_BUS_STATUS_ERR_TIMEOUT,
	I2C_BUS_STATUS_ERR_INVALID_PARAM
} i2c_bus_status_t;

typedef struct __i2c_bus_handle i2c_bus_handle_t;
typedef struct __i2c_bus_ops i2c_bus_ops_t;

struct __i2c_bus_handle {
	void *user_data; 
	i2c_bus_ops_t *ops;
};

struct __i2c_bus_ops {
	i2c_bus_status_t (*init)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*start)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*stop)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*send_ack)(i2c_bus_handle_t *handle, bool ack);
	i2c_bus_status_t (*wait_ack)(i2c_bus_handle_t *handle);
	i2c_bus_status_t (*recv_byte)(i2c_bus_handle_t *handle, uint8_t *byte, bool ack);
	i2c_bus_status_t (*send_byte)(i2c_bus_handle_t *handle, uint8_t byte, bool wait);
	
	i2c_bus_status_t (*read_byte)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte);
	i2c_bus_status_t (*write_byte)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte);
	
	i2c_bus_status_t (*read_bytes)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len);
	i2c_bus_status_t (*write_bytes)(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len);

	i2c_bus_status_t (*read_reg)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte);
	i2c_bus_status_t (*write_reg)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);

	i2c_bus_status_t (*read_regs)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
	i2c_bus_status_t (*write_regs)(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
	
};

//i2c_bus_status_t i2c_start(i2c_bus_handle_t *handle);
//i2c_bus_status_t i2c_stop(i2c_bus_handle_t *handle);
//i2c_bus_status_t i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack);
//i2c_bus_status_t i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte, bool wait);

//i2c_bus_status_t i2c_read_data(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len);
//i2c_bus_status_t i2c_write_data(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len);

//i2c_bus_status_t i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte);
//i2c_bus_status_t i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);

//i2c_bus_status_t i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
//i2c_bus_status_t i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);

static inline i2c_bus_status_t i2c_read_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	if (!handle || !handle->ops || !handle->ops->read_bytes) { return I2C_BUS_STATUS_ERR_INVALID_PARAM; }
	return handle->ops->read_bytes(handle, dev_addr, data, len);
}
static inline i2c_bus_status_t i2c_write_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	if (!handle || !handle->ops || !handle->ops->write_bytes) { return I2C_BUS_STATUS_ERR_INVALID_PARAM; }
	return handle->ops->write_bytes(handle, dev_addr, data, len);
}

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_BUS_H */