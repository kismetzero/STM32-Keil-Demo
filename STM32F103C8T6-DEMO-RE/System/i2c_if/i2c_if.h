#ifndef __I2C_IF_H
#define __I2C_IF_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    I2C_IF_OK = 0,
    I2C_IF_ERR_TIMEOUT,
    I2C_IF_ERR_NACK,
    I2C_IF_ERR_BUS_BUSY,
    I2C_IF_ERR_INVALID_PARAM
} i2c_if_status_t;

// 定义 I2C 设备句柄结构体
typedef struct {
    // 用户自定义数据指针（用于区分不同的实例，如 I2C1, I2C2 或 软模拟引脚组）
    void *user_data; 

    // 核心操作函数指针 (由底层具体实现填充)
    i2c_if_status_t (*init)(void *user_data);
    i2c_if_status_t (*start)(void *user_data);
    i2c_if_status_t (*stop)(void *user_data);
    i2c_if_status_t (*write_byte)(void *user_data, uint8_t data);
    i2c_if_status_t (*read_byte)(void *user_data, uint8_t *data, bool ack);
    
    // 封装好的高层原子操作 (可选，方便直接调用)
    i2c_if_status_t (*mem_write)(void *user_data, uint8_t dev_addr, uint16_t mem_addr, uint8_t *buf, uint16_t len);
    i2c_if_status_t (*mem_read)(void *user_data, uint8_t dev_addr, uint16_t mem_addr, uint8_t *buf, uint16_t len);

} i2c_if_handle_t;

// 通用 API 声明 (应用层只调用这些)
i2c_if_status_t i2c_master_transmit(i2c_if_handle_t *hi2c, uint8_t dev_addr, uint8_t *data, uint16_t size);
i2c_if_status_t i2c_master_receive(i2c_if_handle_t *hi2c, uint8_t dev_addr, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __I2C_IF_H */