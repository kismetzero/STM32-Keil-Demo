#include "i2c_if.h"

i2c_if_status_t i2c_master_transmit(i2c_if_handle_t *hi2c, uint8_t dev_addr, const uint8_t *data, uint16_t size) {
    if (!hi2c || !hi2c->start || !hi2c->write_byte || !hi2c->stop) {
        return I2C_IF_ERR_INVALID_PARAM;
    }

    i2c_if_status_t ret;
    
    // 1. 发送起始信号
    ret = hi2c->start(hi2c->user_data);
    if (ret != I2C_IF_OK) return ret;

    // 2. 发送设备地址 (写位 0)
    ret = hi2c->write_byte(hi2c->user_data, (dev_addr << 1) | 0x00);
    if (ret != I2C_IF_OK) { hi2c->stop(hi2c->user_data); return ret; }

    // 3. 发送数据
    for (uint16_t i = 0; i < size; i++) {
        ret = hi2c->write_byte(hi2c->user_data, data[i]);
        if (ret != I2C_IF_OK) { hi2c->stop(hi2c->user_data); return ret; }
    }

    // 4. 发送停止信号
    return hi2c->stop(hi2c->user_data);
}

i2c_if_status_t i2c_master_receive(i2c_if_handle_t *hi2c, uint8_t dev_addr, uint8_t *data, uint16_t size) {
    if (!hi2c || !hi2c->start || !hi2c->write_byte || !hi2c->read_byte || !hi2c->stop) {
        return I2C_IF_ERR_INVALID_PARAM;
    }

    i2c_if_status_t ret;

    // 1. 发送起始信号
    ret = hi2c->start(hi2c->user_data);
    if (ret != I2C_IF_OK) return ret;

    // 2. 发送设备地址 (读位 1)
    ret = hi2c->write_byte(hi2c->user_data, (dev_addr << 1) | 0x01);
    if (ret != I2C_IF_OK) { hi2c->stop(hi2c->user_data); return ret; }

    // 3. 读取数据
    for (uint16_t i = 0; i < size; i++) {
        bool ack = (i < size - 1); // 最后一个字节发送 NACK
        ret = hi2c->read_byte(hi2c->user_data, &data[i], ack);
        if (ret != I2C_IF_OK) { hi2c->stop(hi2c->user_data); return ret; }
    }

    // 4. 发送停止信号
    return hi2c->stop(hi2c->user_data);
}