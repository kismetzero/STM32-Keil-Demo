#include "i2c_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define LOG_TAG "i2c_impl"
#include "elog.h"

#define I2C_BUS_FAST 1

__STATIC_INLINE void __i2c_delay(void) {
	delay_us(2);
}

__STATIC_INLINE uint8_t __scl_read(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->scl_gpio_port, cfg->scl_gpio_pin);
}

__STATIC_INLINE void __scl_write(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->scl_gpio_port, cfg->scl_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE uint8_t __sda_read(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->sda_gpio_port, cfg->sda_gpio_pin);
}

__STATIC_INLINE void __sda_write(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sda_gpio_port, cfg->sda_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE i2c_bus_status_t __i2c_start(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	// 检查总线是否空闲
	if(!__scl_read(cfg) || !__sda_read(cfg)) {
		return I2C_BUS_STATUS_ERR_BUSY;
	}
	// SDA 从高到低，SCL 保持高
	__sda_write(cfg, 1);
	__scl_write(cfg, 1);
	__i2c_delay();
	// SDA 下降沿
	__sda_write(cfg, 0);
	__i2c_delay();
	// 拉低 SCL，准备发送数据
	__scl_write(cfg, 0);	
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __i2c_stop(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	__sda_write(cfg, 0);
	__scl_write(cfg, 1);
	__i2c_delay();
	// SDA 上升沿
	__sda_write(cfg, 1);
	__i2c_delay();
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __send_ack(i2c_bus_stm32_std_lib_sw_config_t *cfg, bool ack) {
	__scl_write(cfg, 0);
	if (ack) { __sda_write(cfg, 0); }	// SDA = 0 表示应答
	else { __sda_write(cfg, 1); }		// SDA = 1 表示非应答
	__i2c_delay();
	// 时钟高电平，从设备采样ACK
	__scl_write(cfg, 1);
	__i2c_delay();
	
	__scl_write(cfg, 0);
	__sda_write(cfg, 1);
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __wait_ack(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	uint8_t timeout = 0;
	// 释放SDA，让从机拉低
	__sda_write(cfg, 1);
	// 主机释放SCL，等待从机拉低SDA
	__scl_write(cfg, 1);
	__i2c_delay();
	// 循环等待
	while (__sda_read(cfg) && timeout < 254) {
		timeout++;
		__NOP();__NOP();__NOP();
	}
	// 拉低时钟
	__scl_write(cfg, 0);
	__i2c_delay();
	// 超时，无ACK
	if (timeout >= 254) {
		return I2C_BUS_STATUS_ERR_NACK;
	}
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __recv_byte(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t *byte, bool ack) {
	uint8_t res = 0x00;
	__sda_write(cfg, 1);
	for (uint8_t i = 0; i < 8; i++) {
		// 产生时钟上升沿
		__scl_write(cfg, 1);
		__i2c_delay();
		
		res <<= 1;
		if (__sda_read(cfg)) { res |= 0x01; }
		__scl_write(cfg, 0);
		__i2c_delay();
	}
	*byte = res;
	return __send_ack(cfg, ack);
}

__STATIC_INLINE i2c_bus_status_t __send_byte(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t byte, bool wait) {
	for (uint8_t i = 0; i < 8; i++) {
		if (byte & 0x80) { __sda_write(cfg, 1); }
		else { __sda_write(cfg, 0); }
		__i2c_delay();

		__scl_write(cfg, 1);
		__i2c_delay();
		
		__scl_write(cfg, 0);
		if (i == 7) { __sda_write(cfg, 1); }
		
		byte <<= 1;
		__i2c_delay();
	}
	if (wait) {
		return __wait_ack(cfg);
	}
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t SWI2C_Init(i2c_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("SWI2C_Init: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_Init: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	RCC_APB2PeriphClockCmd(cfg->scl_gpio_clk | cfg->sda_gpio_clk, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCL 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->scl_gpio_pin;
	GPIO_Init(cfg->scl_gpio_port, &GPIO_InitStructure);
	// SDA 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->sda_gpio_pin;
	GPIO_Init(cfg->sda_gpio_port, &GPIO_InitStructure);
	// 初始化状态：SCL 和 SDA 高电平（空闲状态）
	__scl_write(cfg, 1);
	__sda_write(cfg, 1);
	log_i("SWI2C_Init: Success! scl_pin=%d, sda_pin=%d", cfg->scl_gpio_pin, cfg->sda_gpio_pin);
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t SWI2C_Start(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 1
	if (handle == NULL || handle->user_data == NULL) {
        return I2C_BUS_STATUS_ERR_INVALID_PARAM;
    }
	return __i2c_start((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_Start: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_Start: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = __i2c_start((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_Start: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("SWI2C_Start: Fail! i2c_bus_busy");
			return ret;
		}
	}
	return ret;
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_Stop(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 1
	return __i2c_stop((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_Stop: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_Stop: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return __i2c_stop((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_SendACK(i2c_bus_handle_t *handle, bool ack) {
#if I2C_BUS_FAST == 1
	return __send_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, ack);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_SendACK: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_SendACK: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return __send_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, ack);
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_WaitACK(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 1
	return __wait_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_WaitACK: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_WaitACK: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = __wait_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_WaitACK: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_e("SWI2C_WaitACK: Fail! NACK Received");
			return ret;
		}
	}
	return ret;
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_RecvByte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack) {
#if I2C_BUS_FAST == 1
	return __recv_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, ack);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_RecvByte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_RecvByte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (byte == NULL) {
		log_e("SWI2C_RecvByte: Fail! byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return __recv_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, ack);
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_SendByte(i2c_bus_handle_t *handle, uint8_t byte, bool wait) {
#if I2C_BUS_FAST == 1
	return __send_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, wait);
#else	/* I2C_BUS_FAST */
	if (handle == NULL) {
		log_e("SWI2C_SendByte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_SendByte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = __send_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, wait);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_SendByte: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("SWI2C_SendByte: Fail! NACK Received");
			return ret;
		}
	}
	return ret;
#endif	/* I2C_BUS_FAST */
}

static i2c_bus_status_t SWI2C_ReadBytes(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL) {
		log_e("SWI2C_ReadData: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_ReadData: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("SWI2C_ReadData: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("SWI2C_ReadData: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __i2c_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_ReadData: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_ReadData: Fail! dev_addr(R)=0x%02X NACK. Device not found?", dev_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	for (uint16_t i = 0; i < len; i++) {
		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
		ret = __recv_byte(cfg, &data[i], ack);
		if (ret != I2C_BUS_STATUS_OK) {
			__i2c_stop(cfg);
			log_e("SWI2C_ReadData: Fail! @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
			return ret;
		}
	}
//	for (uint16_t i = 0; i < len; i++) {
//		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
//		__recv_byte(cfg, &data[i], ack);
//	}
	return __i2c_stop(cfg);
}

static i2c_bus_status_t SWI2C_WriteBytes(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL) {
		log_e("SWI2C_WriteData: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_WriteData: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("SWI2C_WriteData: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("SWI2C_WriteData: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __i2c_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_WriteData: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_WriteData: Fail! @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送数据
	for (uint16_t i = 0; i < len; i++) {
		ret = __send_byte(cfg, data[i], true);
		if (ret != I2C_BUS_STATUS_OK) {
			__i2c_stop(cfg);
			log_e("SWI2C_WriteData: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
			return ret;
		}
	}
	return __i2c_stop(cfg);
}

static i2c_bus_status_t SWI2C_ReadByte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte) {
	return SWI2C_ReadBytes(handle, dev_addr, byte, 1);
}

static i2c_bus_status_t SWI2C_WriteByte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte) {
	return SWI2C_WriteBytes(handle, dev_addr, &byte, 1);
}

static i2c_bus_status_t SWI2C_ReadRegs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if (handle == NULL) {
		log_e("SWI2C_ReadRegs: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_ReadRegs: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("SWI2C_ReadRegs: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("SWI2C_ReadRegs: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __i2c_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_ReadRegs: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_ReadRegs: Fail! @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_ReadRegs: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK", dev_addr, reg_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 重复起始信号
	ret = __i2c_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		// 此时总线状态不确定，尝试发个 Stop 保护一下
		__i2c_stop(cfg);
		log_e("SWI2C_ReadRegs: Fail! @ restart (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_ReadRegs: Fail! dev_addr(R)=0x%02X NACK. Device not found?", dev_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	for (uint16_t i = 0; i < len; i++) {
		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
		ret = __recv_byte(cfg, &data[i], ack);
		if (ret != I2C_BUS_STATUS_OK) {
			__i2c_stop(cfg);
			log_e("SWI2C_ReadRegs: Fail! @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
			return ret;
		}
	}
//	for (uint16_t i = 0; i < len; i++) {
//		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
//		__recv_byte(cfg, &data[i], ack);
//	}
	return __i2c_stop(cfg);
}

static i2c_bus_status_t SWI2C_WriteRegs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
	if (handle == NULL) {
		log_e("SWI2C_WriteRegs: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("SWI2C_WriteRegs: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (data == NULL) {
		log_e("SWI2C_WriteRegs: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("SWI2C_WriteRegs: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __i2c_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("SWI2C_WriteRegs: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_WriteRegs: Fail! @ dev_addr(W)=0x%02X NACK. Device not found?", dev_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__i2c_stop(cfg);
		log_e("SWI2C_WriteRegs: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK", dev_addr, reg_addr);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送数据
	for (uint16_t i = 0; i < len; i++) {
		ret = __send_byte(cfg, data[i], true);
		if (ret != I2C_BUS_STATUS_OK) {
			__i2c_stop(cfg);
			log_e("SWI2C_WriteRegs: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
			return ret;
		}
	}
	return __i2c_stop(cfg);
}

static i2c_bus_status_t SWI2C_ReadReg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte) {
	return SWI2C_ReadRegs(handle, dev_addr, reg_addr, byte, 1);
}

static i2c_bus_status_t SWI2C_WriteReg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
	return SWI2C_WriteRegs(handle, dev_addr, reg_addr, &byte, 1);
}

static i2c_bus_ops_t SWI2C_OPS = {
	.init = SWI2C_Init,
	.start = SWI2C_Start,
	.stop = SWI2C_Stop,
	.send_ack = SWI2C_SendACK,
	.wait_ack = SWI2C_WaitACK,
	.recv_byte = SWI2C_RecvByte,
	.send_byte = SWI2C_SendByte,
	.read_byte = SWI2C_ReadByte,
	.write_byte = SWI2C_WriteByte,
	.read_bytes = SWI2C_ReadBytes,
	.write_bytes = SWI2C_WriteBytes,
	.read_reg = SWI2C_ReadReg,
	.write_reg = SWI2C_WriteReg,
	.read_regs = SWI2C_ReadRegs,
	.write_regs = SWI2C_WriteRegs
};

i2c_bus_status_t i2c_bus_stm32_std_lib_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	if (handle == NULL) {
		log_e("i2c_bus_stm32_std_lib_sw_create_handle: Fail handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("i2c_bus_stm32_std_lib_sw_create_handle: Fail cfg == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &SWI2C_OPS;
	i2c_bus_status_t ret = handle->ops->init(handle);
	if (ret == I2C_BUS_STATUS_OK) {
		log_i("i2c_bus_stm32_std_lib_sw_create_handle: Init Handle Success");
	} else {
		log_e("i2c_bus_stm32_std_lib_sw_create_handle: Fail Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}
