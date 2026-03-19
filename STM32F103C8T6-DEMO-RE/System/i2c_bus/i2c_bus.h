#ifndef __I2C_BUS_H
#define __I2C_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
	I2C_BUS_OK = 0,
	I2C_BUS_ERR_BUSY,
	I2C_BUS_ERR_DEV_NONE,
	I2C_BUS_ERR_NACK,
	I2C_BUS_ERR_TIMEOUT,
	I2C_BUS_ERR_INVALID_PARAM
} i2c_bus_status_t;

// 定义 I2C 总线操作结构体
typedef struct {
	// 核心操作函数指针 (由底层具体实现填充)
	i2c_bus_status_t (*init)(void *user_data);
	i2c_bus_status_t (*start)(void *user_data);
	i2c_bus_status_t (*stop)(void *user_data);
	i2c_bus_status_t (*send_ack)(void *user_data, bool ack);
	i2c_bus_status_t (*wait_ack)(void *user_data);
	i2c_bus_status_t (*recv_byte)(void *user_data, uint8_t *byte, bool ack);
	i2c_bus_status_t (*send_byte)(void *user_data, uint8_t byte, bool wait);
	
//	i2c_bus_status_t (*read_data)(void *user_data, uint8_t dev_addr, uint8_t *data, uint16_t len);
//	i2c_bus_status_t (*write_data)(void *user_data, uint8_t dev_addr, const uint8_t *data, uint16_t len);

//	i2c_bus_status_t (*read_reg)(void *user_data, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte);
//	i2c_bus_status_t (*write_reg)(void *user_data, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);

//	i2c_bus_status_t (*read_regs)(void *user_data, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
//	i2c_bus_status_t (*write_regs)(void *user_data, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
	
} i2c_bus_ops_t;

// 定义 I2C 总线句柄结构体
typedef struct {
	// 用户自定义数据指针（用于区分不同的实例，如 I2C1, I2C2 或 软模拟引脚组）
	void *user_data; 
	i2c_bus_ops_t *ops;
} i2c_bus_handle_t;

i2c_bus_status_t i2c_start(i2c_bus_handle_t *handle);
i2c_bus_status_t i2c_stop(i2c_bus_handle_t *handle);
i2c_bus_status_t i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack);
i2c_bus_status_t i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte, bool wait);

i2c_bus_status_t i2c_read_data(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len);
i2c_bus_status_t i2c_write_data(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len);

i2c_bus_status_t i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte);
i2c_bus_status_t i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte);

i2c_bus_status_t i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
i2c_bus_status_t i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_BUS_H */