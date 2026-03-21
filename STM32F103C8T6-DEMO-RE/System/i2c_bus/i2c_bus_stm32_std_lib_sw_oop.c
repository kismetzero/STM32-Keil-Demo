#include "i2c_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define LOG_TAG "i2c_impl"
#include "elog.h"

#define I2C_BUS_FAST 1

__STATIC_INLINE void __delay(void) {
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

__STATIC_INLINE i2c_bus_status_t __start(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	// 检查总线是否空闲
	if(!__scl_read(cfg) || !__sda_read(cfg)) {
		return I2C_BUS_STATUS_ERR_BUSY;
	}
	// SDA 从高到低，SCL 保持高
	__sda_write(cfg, 1);
	__scl_write(cfg, 1);
	__delay();
	// SDA 下降沿
	__sda_write(cfg, 0);
	__delay();
	// 拉低 SCL，准备发送数据
	__scl_write(cfg, 0);	
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __stop(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	__sda_write(cfg, 0);
	__scl_write(cfg, 1);
	__delay();
	// SDA 上升沿
	__sda_write(cfg, 1);
	__delay();
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t __send_ack(i2c_bus_stm32_std_lib_sw_config_t *cfg, bool ack) {
	__scl_write(cfg, 0);
	if (ack) { __sda_write(cfg, 0); }	// SDA = 0 表示应答
	else { __sda_write(cfg, 1); }		// SDA = 1 表示非应答
	__delay();
	// 时钟高电平，从设备采样ACK
	__scl_write(cfg, 1);
	__delay();
	
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
	__delay();
	// 循环等待
	while (__sda_read(cfg) && timeout < 254) {
		timeout++;
		__NOP();__NOP();__NOP();
	}
	// 拉低时钟
	__scl_write(cfg, 0);
	__delay();
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
		__delay();
		
		res <<= 1;
		if (__sda_read(cfg)) { res |= 0x01; }
		__scl_write(cfg, 0);
		__delay();
	}
	*byte = res;
	return __send_ack(cfg, ack);
}

__STATIC_INLINE i2c_bus_status_t __send_byte(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t byte, bool wait) {
	for (uint8_t i = 0; i < 8; i++) {
		if (byte & 0x80) { __sda_write(cfg, 1); }
		else { __sda_write(cfg, 0); }
		__delay();

		__scl_write(cfg, 1);
		__delay();
		
		__scl_write(cfg, 0);
		if (i == 7) { __sda_write(cfg, 1); }
		
		byte <<= 1;
		__delay();
	}
	if (wait) {
		return __wait_ack(cfg);
	}
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t __i2c_init(i2c_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("__i2c_init: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_init: Fail! user_data == NULL");
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
	log_i("__i2c_init: Success! scl_pin=%d, sda_pin=%d", cfg->scl_gpio_pin, cfg->sda_gpio_pin);
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t __i2c_start(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_start: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_start: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	i2c_bus_status_t ret = __start((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_start: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("__i2c_start: Fail! i2c_bus_busy");
			return ret;
		}
	}
	return ret;
}

static i2c_bus_status_t __i2c_stop(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_stop: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_stop: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	return __stop((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
}

static i2c_bus_status_t __i2c_send_ack(i2c_bus_handle_t *handle, bool ack) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_send_ack: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_send_ack: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	return __send_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, ack);
}

static i2c_bus_status_t __i2c_wait_ack(i2c_bus_handle_t *handle) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_wait_ack: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_wait_ack: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	i2c_bus_status_t ret = __wait_ack((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_wait_ack: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_e("__i2c_wait_ack: Fail! NACK Received");
			return ret;
		}
	}
	return ret;
}

static i2c_bus_status_t __i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte, bool ack) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_recv_byte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_recv_byte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (byte == NULL) {
		log_e("__i2c_recv_byte: Fail! byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return __recv_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, ack);
}

static i2c_bus_status_t __i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte, bool wait) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_send_byte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_send_byte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	i2c_bus_status_t ret = __send_byte((i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data, byte, wait);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_send_byte: Fail! (Code: %d)", ret);
		if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_e("__i2c_send_byte: Fail! NACK Received");
			return ret;
		}
	}
	return ret;
}

static i2c_bus_status_t __i2c_read_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_read_bytes: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_read_bytes: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (data == NULL) {
		log_e("__i2c_read_bytes: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("__i2c_read_bytes: Fail len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_read_bytes: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_bytes: Fail! dev_addr(R)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	for (uint16_t i = 0; i < len; i++) {
		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
		ret = __recv_byte(cfg, &data[i], ack);
		if (ret != I2C_BUS_STATUS_OK) {
			__stop(cfg);
			log_e("__i2c_read_bytes: Fail! @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
			return ret;
		}
	}
//	for (uint16_t i = 0; i < len; i++) {
//		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
//		__recv_byte(cfg, &data[i], ack);
//	}
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_write_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_write_bytes: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_write_bytes: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (data == NULL) {
		log_e("__i2c_write_bytes: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("__i2c_write_bytes: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_write_bytes: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_bytes: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送数据
	for (uint16_t i = 0; i < len; i++) {
		ret = __send_byte(cfg, data[i], true);
		if (ret != I2C_BUS_STATUS_OK) {
			__stop(cfg);
			log_e("__i2c_write_bytes: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
			return ret;
		}
	}
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_read_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_read_byte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_read_byte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (byte == NULL) {
		log_e("__i2c_read_byte: Fail! byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_read_byte: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_byte: Fail! dev_addr(R)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	ret = __recv_byte(cfg, byte, false);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_byte: Fail! @ dev_addr(R)=0x%02X recv_byte (Code: %d)", dev_addr, ret);
		return ret;
	}
//	__recv_byte(cfg, byte, false);
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_write_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_write_byte: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_write_byte: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_write_byte: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_byte: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送数据
	ret = __send_byte(cfg, byte, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_byte: Fail @ dev_addr(W)=0x%02X byte=0x%02X NACK (Code: %d)", dev_addr, byte, ret);
		return ret;
	}
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_read_regs: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_read_regs: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (data == NULL) {
		log_e("__i2c_read_regs: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("__i2c_read_regs: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_read_regs: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_regs: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_regs: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK (Code: %d)", dev_addr, reg_addr, ret);
		return I2C_BUS_STATUS_ERR_REG_NONE;
	}
	// 重复起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		// 此时总线状态不确定，尝试发个 Stop 保护一下
		__stop(cfg);
		log_e("__i2c_read_regs: Fail! @ restart (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_regs: Fail! dev_addr(R)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	for (uint16_t i = 0; i < len; i++) {
		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
		ret = __recv_byte(cfg, &data[i], ack);
		if (ret != I2C_BUS_STATUS_OK) {
			__stop(cfg);
			log_e("__i2c_read_regs: Fail! @ dev_addr(R)=0x%02X DATA[%d] (Code: %d)", dev_addr, i, ret);
			return ret;
		}
	}
//	for (uint16_t i = 0; i < len; i++) {
//		bool ack = (i < len - 1);	// 最后一个字节发送 NACK
//		__recv_byte(cfg, &data[i], ack);
//	}
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_write_regs: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_write_regs: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (data == NULL) {
		log_e("__i2c_write_regs: Fail! data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("__i2c_write_regs: Fail! len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_write_regs: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_regs: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_regs: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK (Code: %d)", dev_addr, reg_addr, ret);
		return I2C_BUS_STATUS_ERR_REG_NONE;
	}
	// 发送数据
	for (uint16_t i = 0; i < len; i++) {
		ret = __send_byte(cfg, data[i], true);
		if (ret != I2C_BUS_STATUS_OK) {
			__stop(cfg);
			log_e("__i2c_write_regs: Fail @ dev_addr(W)=0x%02X DATA[%d]=0x%02X (Code: %d)", dev_addr, i, data[i], ret);
			return ret;
		}
	}
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_read_reg: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_read_reg: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	if (byte == NULL) {
		log_e("__i2c_read_reg: Fail! byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_read_reg: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_reg: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_reg: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK (Code: %d)", dev_addr, reg_addr, ret);
		return I2C_BUS_STATUS_ERR_REG_NONE;
	}
	// 重复起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		// 此时总线状态不确定，尝试发个 Stop 保护一下
		__stop(cfg);
		log_e("__i2c_read_reg: Fail! @ restart (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x01, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_reg: Fail! dev_addr(R)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 读取数据
	ret = __recv_byte(cfg, byte, false);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_read_reg: Fail! @ dev_addr(R)=0x%02X recv_byte (Code: %d)", dev_addr, ret);
		return ret;
	}
//	__recv_byte(cfg, byte, ack);
	return __stop(cfg);
}

static i2c_bus_status_t __i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__i2c_write_reg: Fail! handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__i2c_write_reg: Fail! user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* I2C_BUS_FAST */
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret;
	// 发送起始信号
	ret = __start(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("__i2c_write_reg: Fail! @ start (Code: %d). Bus busy?", ret);
		return ret;
	}
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	ret = __send_byte(cfg, (dev_addr << 1) | 0x00, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_reg: Fail! @ dev_addr(W)=0x%02X NACK. Device not found? (Code: %d)", dev_addr, ret);
		return I2C_BUS_STATUS_ERR_DEV_NONE;
	}
	// 发送寄存器地址
	ret = __send_byte(cfg, reg_addr, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_reg: Fail! @ dev_addr(W)=0x%02X RegAddr=0x%02X NACK (Code: %d)", dev_addr, reg_addr, ret);
		return I2C_BUS_STATUS_ERR_REG_NONE;
	}
	// 发送数据
	ret = __send_byte(cfg, byte, true);
	if (ret != I2C_BUS_STATUS_OK) {
		__stop(cfg);
		log_e("__i2c_write_reg: Fail @ dev_addr(W)=0x%02X byte=0x%02X NACK (Code: %d)", dev_addr, byte, ret);
		return ret;
	}
	return __stop(cfg);
}

static i2c_bus_ops_t __i2c_ops = {
	.init = __i2c_init,
	.start = __i2c_start,
	.stop = __i2c_stop,
	.send_ack = __i2c_send_ack,
	.wait_ack = __i2c_wait_ack,
	.recv_byte = __i2c_recv_byte,
	.send_byte = __i2c_send_byte,
	.read_byte = __i2c_read_byte,
	.write_byte = __i2c_write_byte,
	.read_bytes = __i2c_read_bytes,
	.write_bytes = __i2c_write_bytes,
	.read_reg = __i2c_read_reg,
	.write_reg = __i2c_write_reg,
	.read_regs = __i2c_read_regs,
	.write_regs = __i2c_write_regs
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
	handle->ops = &__i2c_ops;
	i2c_bus_status_t ret = handle->ops->init(handle);
	if (ret == I2C_BUS_STATUS_OK) {
		log_i("i2c_bus_stm32_std_lib_sw_create_handle: Init Handle Success");
	} else {
		log_e("i2c_bus_stm32_std_lib_sw_create_handle: Fail Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}
