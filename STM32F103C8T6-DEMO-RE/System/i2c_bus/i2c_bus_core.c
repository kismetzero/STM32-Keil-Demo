#include "i2c_bus.h"

i2c_bus_status_t i2c_bus_transmit(i2c_bus_handle_t *hi2c, uint8_t dev_addr, const uint8_t *data, uint16_t size) {
    if (!hi2c || !hi2c->ops) {
        return I2C_BUS_ERR_INVALID_PARAM;
    }

    i2c_bus_status_t ret;
    
    // 1. 发送起始信号
    ret = hi2c->ops->start(hi2c->user_data);
    if (ret != I2C_BUS_OK) return ret;

    // 2. 发送设备地址 (写位 0)
    ret = hi2c->ops->write_byte(hi2c->user_data, (dev_addr << 1) | 0x00);
    if (ret != I2C_BUS_OK) { hi2c->ops->stop(hi2c->user_data); return ret; }

    // 3. 发送数据
    for (uint16_t i = 0; i < size; i++) {
        ret = hi2c->ops->write_byte(hi2c->user_data, data[i]);
        if (ret != I2C_BUS_OK) { hi2c->ops->stop(hi2c->user_data); return ret; }
    }

    // 4. 发送停止信号
    return hi2c->ops->stop(hi2c->user_data);
}

i2c_bus_status_t i2c_bus_receive(i2c_bus_handle_t *hi2c, uint8_t dev_addr, uint8_t *data, uint16_t size) {
    if (!hi2c || !hi2c->ops) {
        return I2C_BUS_ERR_INVALID_PARAM;
    }

    i2c_bus_status_t ret;

    // 1. 发送起始信号
    ret = hi2c->ops->start(hi2c->user_data);
    if (ret != I2C_BUS_OK) return ret;

    // 2. 发送设备地址 (读位 1)
    ret = hi2c->ops->write_byte(hi2c->user_data, (dev_addr << 1) | 0x01);
    if (ret != I2C_BUS_OK) { hi2c->ops->stop(hi2c->user_data); return ret; }

    // 3. 读取数据
    for (uint16_t i = 0; i < size; i++) {
        bool ack = (i < size - 1); // 最后一个字节发送 NACK
        ret = hi2c->ops->read_byte(hi2c->user_data, &data[i], ack);
        if (ret != I2C_BUS_OK) { hi2c->ops->stop(hi2c->user_data); return ret; }
    }

    // 4. 发送停止信号
    return hi2c->ops->stop(hi2c->user_data);
}