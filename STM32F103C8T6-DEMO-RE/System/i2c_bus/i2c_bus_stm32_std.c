#include "i2c_bus_stm32_std.h"
#include "delay.h"

#define LOG_TAG "i2c_impl"
#include "elog.h"

#define I2C_BUS_FAST 0

//=====================================================================================================
// I2C SW BUS OOP

__STATIC_INLINE void i2c_bus_stm32_std_sw_delay(void) {
	delay_us(2);
}

__STATIC_INLINE uint8_t i2c_bus_stm32_std_sw_scl_read(i2c_bus_stm32_std_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->scl_gpio_port, cfg->scl_gpio_pin);
}

__STATIC_INLINE void i2c_bus_stm32_std_sw_scl_write(i2c_bus_stm32_std_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->scl_gpio_port, cfg->scl_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE uint8_t i2c_bus_stm32_std_sw_sda_read(i2c_bus_stm32_std_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->sda_gpio_port, cfg->sda_gpio_pin);
}

__STATIC_INLINE void i2c_bus_stm32_std_sw_sda_write(i2c_bus_stm32_std_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sda_gpio_port, cfg->sda_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_start(i2c_bus_stm32_std_sw_config_t *cfg) {
	// SDA 从高到低，SCL 保持高
	i2c_bus_stm32_std_sw_sda_write(cfg, 1);
	i2c_bus_stm32_std_sw_scl_write(cfg, 1);
	i2c_bus_stm32_std_sw_delay();
	// 检查总线是否空闲
	if(!i2c_bus_stm32_std_sw_scl_read(cfg) || !i2c_bus_stm32_std_sw_sda_read(cfg)) {
		return I2C_BUS_STATUS_ERR_BUSY;
	}
	// SDA 下降沿
	i2c_bus_stm32_std_sw_sda_write(cfg, 0);
	i2c_bus_stm32_std_sw_delay();
	// 拉低 SCL，准备发送数据
	i2c_bus_stm32_std_sw_scl_write(cfg, 0);	
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_stop(i2c_bus_stm32_std_sw_config_t *cfg) {
	i2c_bus_stm32_std_sw_sda_write(cfg, 0);
	i2c_bus_stm32_std_sw_scl_write(cfg, 1);
	i2c_bus_stm32_std_sw_delay();
	// SDA 上升沿
	i2c_bus_stm32_std_sw_sda_write(cfg, 1);
	i2c_bus_stm32_std_sw_delay();
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_send_ack(i2c_bus_stm32_std_sw_config_t *cfg, bool ack) {
	if (ack) { i2c_bus_stm32_std_sw_sda_write(cfg, 0); }	// SDA = 0 表示应答
	else { i2c_bus_stm32_std_sw_sda_write(cfg, 1); }		// SDA = 1 表示非应答
	i2c_bus_stm32_std_sw_delay();
	// 时钟高电平，从设备采样ACK
	i2c_bus_stm32_std_sw_scl_write(cfg, 1);
	i2c_bus_stm32_std_sw_delay();
	
	i2c_bus_stm32_std_sw_scl_write(cfg, 0);
	i2c_bus_stm32_std_sw_delay();
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_wait_ack(i2c_bus_stm32_std_sw_config_t *cfg) {
	uint8_t timeout = 0;
	// 释放SDA，让从机拉低
	i2c_bus_stm32_std_sw_sda_write(cfg, 1);
	// 主机释放SCL，等待从机拉低SDA
	i2c_bus_stm32_std_sw_scl_write(cfg, 1);
	i2c_bus_stm32_std_sw_delay();
	// 循环等待
	while (i2c_bus_stm32_std_sw_sda_read(cfg) && timeout < 254) {
		timeout++;
		__NOP();__NOP();__NOP();
	}
	// 拉低时钟
	i2c_bus_stm32_std_sw_scl_write(cfg, 0);
	// 超时，无ACK
	if (timeout >= 254) {
		return I2C_BUS_STATUS_ERR_NACK;
	}
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_recv_byte(i2c_bus_stm32_std_sw_config_t *cfg, uint8_t *byte) {
	uint8_t res = 0x00;
	i2c_bus_stm32_std_sw_sda_write(cfg, 1);
	for (uint8_t i = 0; i < 8; i++) {
		// 产生时钟上升沿
		i2c_bus_stm32_std_sw_scl_write(cfg, 1);
		i2c_bus_stm32_std_sw_delay();
		
		if (i2c_bus_stm32_std_sw_sda_read(cfg)) { res |= (0x80 >> i); }
		i2c_bus_stm32_std_sw_scl_write(cfg, 0);
		i2c_bus_stm32_std_sw_delay();
	}
	*byte = res;
	return I2C_BUS_STATUS_OK;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_send_byte(i2c_bus_stm32_std_sw_config_t *cfg, uint8_t byte) {
	for (uint8_t i = 0; i < 8; i++) {
		if (byte & (0x80 >> i)) { i2c_bus_stm32_std_sw_sda_write(cfg, 1); }
		else { i2c_bus_stm32_std_sw_sda_write(cfg, 0); }
		i2c_bus_stm32_std_sw_delay();

		i2c_bus_stm32_std_sw_scl_write(cfg, 1);
		i2c_bus_stm32_std_sw_delay();
		
		i2c_bus_stm32_std_sw_scl_write(cfg, 0);
		i2c_bus_stm32_std_sw_delay();
	}
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_master_trans(i2c_bus_stm32_std_sw_config_t *cfg, uint8_t dev_addr, 
	const uint8_t *tx_buf, uint16_t tx_len, uint8_t *rx_buf, uint16_t rx_len) {
	if (tx_len == 0 && rx_len == 0) { return I2C_BUS_STATUS_ERR_INVALID_PARAM; }
	#if SYS_EN_FREERTOS
		if (xSemaphoreTakeRecursive(cfg->mutex_lock, pdMS_TO_TICKS(1000)) != pdTRUE) {
			log_w("mutex_lock timeout");
			return I2C_BUS_STATUS_ERR_BUSY;
		}
	#endif /* SYS_EN_FREERTOS */
	i2c_bus_status_t ret = I2C_BUS_STATUS_OK;
	if (tx_len > 0) {
		// 发送起始信号
		ret = i2c_bus_stm32_std_sw_start(cfg);
		if (ret != I2C_BUS_STATUS_OK) { goto exit; }
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		i2c_bus_stm32_std_sw_send_byte(cfg, (dev_addr << 1) | 0x00);
		ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
		if (ret != I2C_BUS_STATUS_OK) {
			ret = I2C_BUS_STATUS_ERR_DEV_NONE;
			goto stop;
		}
		// 发送数据
		for (uint16_t i = 0; i < tx_len; i++) {
			i2c_bus_stm32_std_sw_send_byte(cfg, tx_buf[i]);
			ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
			if (ret != I2C_BUS_STATUS_OK) {
				log_w("dev_addr=0x%02X write data[%d]=0x%02X (code: %d)", dev_addr, i, tx_buf[i], ret);
				goto stop;
			}
		}
	}
	if (rx_len > 0) {
		// 发送起始信号
		ret = i2c_bus_stm32_std_sw_start(cfg);
		if (ret != I2C_BUS_STATUS_OK) { goto exit; }
		// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
		i2c_bus_stm32_std_sw_send_byte(cfg, (dev_addr << 1) | 0x01);
		ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
		if (ret != I2C_BUS_STATUS_OK) {
			ret = I2C_BUS_STATUS_ERR_DEV_NONE;
			goto stop;
		}
		// 发送数据
		for (uint16_t i = 0; i < rx_len; i++) {
			i2c_bus_stm32_std_sw_recv_byte(cfg, &rx_buf[i]);
			i2c_bus_stm32_std_sw_send_ack(cfg, i < rx_len - 1);
		}
	}
	stop:
	i2c_bus_stm32_std_sw_stop(cfg);
	exit:
	#if SYS_EN_FREERTOS
		xSemaphoreGiveRecursive(cfg->mutex_lock);
	#endif /* SYS_EN_FREERTOS */
	return ret;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_init(i2c_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited == 1) {
		log_d("already init");
		return I2C_BUS_STATUS_OK;
	}
	#if SYS_EN_FREERTOS
		if (cfg->mutex_lock == NULL) {
			cfg->mutex_lock = xSemaphoreCreateRecursiveMutex();
			if (cfg->mutex_lock == NULL) {
				log_e("mutex_lock create fail");
				return I2C_BUS_STATUS_ERR;
			}
		}
	#endif /* SYS_EN_FREERTOS */
	// 使能 GPIO 总线时钟
	RCC_APB2PeriphClockCmd(cfg->scl_gpio_clk | cfg->sda_gpio_clk, ENABLE);
	// GPIO 初始化
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
	i2c_bus_stm32_std_sw_scl_write(cfg, 1);
	i2c_bus_stm32_std_sw_sda_write(cfg, 1);
	log_d("success! (scl_pin=%d, sda_pin=%d)", cfg->scl_gpio_pin, cfg->sda_gpio_pin);
	cfg->inited = 1;
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_deinit(i2c_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		return I2C_BUS_STATUS_OK;
	}
	#if SYS_EN_FREERTOS
		if (cfg->mutex_lock != NULL) {
			vSemaphoreDelete(cfg->mutex_lock);
            cfg->mutex_lock = NULL;
		}
	#endif /* SYS_EN_FREERTOS */
	cfg->inited = 0;
	return I2C_BUS_STATUS_OK;
}

#if I2C_BUS_SIMP == 0 || !defined(I2C_BUS_SIMP)
static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_lock(i2c_bus_handle_t *handle) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	#if SYS_EN_FREERTOS
		if (xSemaphoreTakeRecursive(cfg->mutex_lock, pdMS_TO_TICKS(1000)) != pdTRUE) {
			log_w("mutex_lock timeout");
			return I2C_BUS_STATUS_ERR_BUSY;
		}
	#endif /* SYS_EN_FREERTOS */
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_unlock(i2c_bus_handle_t *handle) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	#if SYS_EN_FREERTOS
		xSemaphoreGiveRecursive(cfg->mutex_lock);
	#endif /* SYS_EN_FREERTOS */
	return I2C_BUS_STATUS_OK;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_start(i2c_bus_handle_t *handle) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	return i2c_bus_stm32_std_sw_start(cfg);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_stop(i2c_bus_handle_t *handle) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	return i2c_bus_stm32_std_sw_stop(cfg);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_send_ack(i2c_bus_handle_t *handle, bool ack) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	return i2c_bus_stm32_std_sw_send_ack(cfg, ack);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_wait_ack(i2c_bus_handle_t *handle) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	return i2c_bus_stm32_std_sw_wait_ack(cfg);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_recv_byte(i2c_bus_handle_t *handle, uint8_t *byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	if (byte == NULL) {
		log_e("byte == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	return i2c_bus_stm32_std_sw_recv_byte(cfg, byte);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_send_byte(i2c_bus_handle_t *handle, uint8_t byte) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	return i2c_bus_stm32_std_sw_send_byte(cfg, byte);
}
#endif /* I2C_BUS_SIMP */

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_master_trans(i2c_bus_handle_t *handle, uint8_t dev_addr, 
	const uint8_t *tx_buf, uint16_t tx_len, uint8_t *rx_buf, uint16_t rx_len) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_master_trans(cfg, dev_addr, tx_buf, tx_len, rx_buf, rx_len);
	if (ret != I2C_BUS_STATUS_OK) {
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("bus busy (code: %d)", ret);
		} else if (ret == I2C_BUS_STATUS_ERR_DEV_NONE) {
			log_e("dev(0x%02X) not find (code: %d)", dev_addr, ret);
		} else if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_w("dev(0x%02X) write data fail (code: %d)", dev_addr, ret);
		} else {
			log_e("unknown erro (code: %d)", ret);
		}
	}
	return ret;
}

#if I2C_BUS_SIMP <= 1 || !defined(I2C_BUS_SIMP)
static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_read_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	if (data == NULL) {
		log_e("data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_master_trans(cfg, dev_addr, NULL, 0, data, len);
	if (ret != I2C_BUS_STATUS_OK) {
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("bus busy (code: %d)", ret);
		} else if (ret == I2C_BUS_STATUS_ERR_DEV_NONE) {
			log_e("dev(0x%02X) not find (code: %d)", dev_addr, ret);
		} else {
			log_e("unknown erro (code: %d)", ret);
		}
	}
	return ret;
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_write_bytes(i2c_bus_handle_t *handle, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	if (data == NULL) {
		log_e("data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_master_trans(cfg, dev_addr, data, len, NULL, 0);
	if (ret != I2C_BUS_STATUS_OK) {
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("bus busy (code: %d)", ret);
		} else if (ret == I2C_BUS_STATUS_ERR_DEV_NONE) {
			log_e("dev(0x%02X) not find (code: %d)", dev_addr, ret);
		} else if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_w("dev(0x%02X) write data fail (code: %d)", dev_addr, ret);
		} else {
			log_e("unknown erro (code: %d)", ret);
		}
	}
	return ret;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_read_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t *byte) {
	return i2c_bus_stm32_std_sw_i2c_read_bytes(handle, dev_addr, byte, 1);
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_write_byte(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t byte) {
	return i2c_bus_stm32_std_sw_i2c_write_bytes(handle, dev_addr, &byte, 1);
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_read_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	if (data == NULL) {
		log_e("data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_master_trans(cfg, dev_addr, &reg_addr, 1, data, len);
	if (ret != I2C_BUS_STATUS_OK) {
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("bus busy (code: %d)", ret);
		} else if (ret == I2C_BUS_STATUS_ERR_DEV_NONE) {
			log_e("dev(0x%02X) not find (code: %d)", dev_addr, ret);
		} else if (ret == I2C_BUS_STATUS_ERR_NACK) {
			log_w("dev(0x%02X) write reg(0x%02X) fail (code: %d)", dev_addr, reg_addr, ret);
			return I2C_BUS_STATUS_ERR_REG_NONE;
		} else {
			log_e("unknown erro (code: %d)", ret);
		}
	}
	return ret;
	
}

static i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_write_regs(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
	#if I2C_BUS_FAST == 0
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* I2C_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("user_data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	i2c_bus_stm32_std_sw_config_t *cfg = (i2c_bus_stm32_std_sw_config_t *)handle->user_data;
	if (cfg->inited != 1) {
		log_e("no init");
		return I2C_BUS_STATUS_ERR_NO_INIT;
	}
	if (data == NULL) {
		log_e("data == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("len == 0");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#if SYS_EN_FREERTOS
		if (xSemaphoreTakeRecursive(cfg->mutex_lock, pdMS_TO_TICKS(1000)) != pdTRUE) {
			log_w("mutex_lock timeout");
			return I2C_BUS_STATUS_ERR_BUSY;
		}
	#endif /* SYS_EN_OS */
	i2c_bus_status_t ret = I2C_BUS_STATUS_OK;
	// 发送起始信号
	ret = i2c_bus_stm32_std_sw_start(cfg);
	if (ret != I2C_BUS_STATUS_OK) { goto exit; }
	// 发送设备地址 (写(W): 0x00; 读(R): 0x01)
	i2c_bus_stm32_std_sw_send_byte(cfg, (dev_addr << 1) | 0x00);
	ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		ret = I2C_BUS_STATUS_ERR_DEV_NONE;
		goto stop;
	}
	// 发送寄存器地址
	i2c_bus_stm32_std_sw_send_byte(cfg, reg_addr);
	ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		ret = I2C_BUS_STATUS_ERR_REG_NONE;
		goto stop;
	}
	// 发送数据
	for (uint16_t i = 0; i < len; i++) {
		i2c_bus_stm32_std_sw_send_byte(cfg, data[i]);
		ret = i2c_bus_stm32_std_sw_wait_ack(cfg);
		if (ret != I2C_BUS_STATUS_OK) {
			log_w("dev(0x%02X) write data[%d]=0x%02X (code: %d)", dev_addr, i, data[i], ret);
			goto stop;
		}
	}
	stop:
	i2c_bus_stm32_std_sw_stop(cfg);
	exit:
	#if SYS_EN_FREERTOS
		xSemaphoreGiveRecursive(cfg->mutex_lock);
	#endif /* SYS_EN_OS */
	if (ret != I2C_BUS_STATUS_OK) {
		if (ret == I2C_BUS_STATUS_ERR_BUSY) {
			log_e("bus busy (code: %d)", ret);
		} else if (ret == I2C_BUS_STATUS_ERR_DEV_NONE) {
			log_e("dev(0x%02X) not find (code: %d)", dev_addr, ret);
		} else if (ret == I2C_BUS_STATUS_ERR_REG_NONE) {
			log_w("dev(0x%02X) write reg(0x%02X) fail (code: %d)", dev_addr, reg_addr, ret);
		} else {
			log_e("unknown erro (code: %d)", ret);
		}
	}
	return ret;
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_read_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t *byte) {
	return i2c_bus_stm32_std_sw_i2c_read_regs(handle, dev_addr, reg_addr, byte, 1);
}

__STATIC_INLINE i2c_bus_status_t i2c_bus_stm32_std_sw_i2c_write_reg(i2c_bus_handle_t *handle, uint8_t dev_addr, uint8_t reg_addr, uint8_t byte) {
	return i2c_bus_stm32_std_sw_i2c_write_regs(handle, dev_addr, reg_addr, &byte, 1);
}
#endif /* I2C_BUS_SIMP */

static const i2c_bus_ops_t i2c_bus_stm32_std_sw_ops = {
	.init = i2c_bus_stm32_std_sw_i2c_init,
	.deinit = i2c_bus_stm32_std_sw_i2c_deinit,

#if I2C_BUS_SIMP == 0 || !defined(I2C_BUS_SIMP)
	.lock = i2c_bus_stm32_std_sw_i2c_lock,
	.unlock = i2c_bus_stm32_std_sw_i2c_unlock,
	
	.start = i2c_bus_stm32_std_sw_i2c_start,
	.stop = i2c_bus_stm32_std_sw_i2c_stop,
	.send_ack = i2c_bus_stm32_std_sw_i2c_send_ack,
	.wait_ack = i2c_bus_stm32_std_sw_i2c_wait_ack,
	.recv_byte = i2c_bus_stm32_std_sw_i2c_recv_byte,
	.send_byte = i2c_bus_stm32_std_sw_i2c_send_byte,
#endif /* I2C_BUS_SIMP */
	
	.master_trans = i2c_bus_stm32_std_sw_i2c_master_trans,

#if I2C_BUS_SIMP <= 1 || !defined(I2C_BUS_SIMP)
	.read_byte = i2c_bus_stm32_std_sw_i2c_read_byte,
	.write_byte = i2c_bus_stm32_std_sw_i2c_write_byte,
	.read_bytes = i2c_bus_stm32_std_sw_i2c_read_bytes,
	.write_bytes = i2c_bus_stm32_std_sw_i2c_write_bytes,
	
	.read_reg = i2c_bus_stm32_std_sw_i2c_read_reg,
	.write_reg = i2c_bus_stm32_std_sw_i2c_write_reg,
	.read_regs = i2c_bus_stm32_std_sw_i2c_read_regs,
	.write_regs = i2c_bus_stm32_std_sw_i2c_write_regs,
#endif /* I2C_BUS_SIMP */
};

i2c_bus_status_t i2c_bus_stm32_std_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_sw_config_t *cfg) {
	if (handle == NULL) {
		log_e("handle == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("cfg == NULL");
		return I2C_BUS_STATUS_ERR_INVALID_PARAM;
	}
	
	assert(IS_RCC_APB2_PERIPH(cfg->scl_gpio_clk));
	assert(IS_GPIO_PIN(cfg->scl_gpio_pin));
	assert(IS_GPIO_ALL_PERIPH(cfg->scl_gpio_port));
	
	assert(IS_RCC_APB2_PERIPH(cfg->sda_gpio_clk));
	assert(IS_GPIO_PIN(cfg->sda_gpio_pin));
	assert(IS_GPIO_ALL_PERIPH(cfg->sda_gpio_port));
	
	handle->user_data = cfg;
	handle->ops = &i2c_bus_stm32_std_sw_ops;
	
	i2c_bus_status_t ret = handle->ops->init(handle);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("init fail (code: %d)", ret);
	}
	return ret;
}

// #define I2C_BUS_STM32_STD_SW_SCL_GPIO_CLK		RCC_APB2Periph_GPIOB
// #define I2C_BUS_STM32_STD_SW_SCL_GPIO_PIN		GPIO_Pin_8
// #define I2C_BUS_STM32_STD_SW_SCL_GPIO_PORT		GPIOB

// #define I2C_BUS_STM32_STD_SW_SDA_GPIO_CLK		RCC_APB2Periph_GPIOB
// #define I2C_BUS_STM32_STD_SW_SDA_GPIO_PIN		GPIO_Pin_9
// #define I2C_BUS_STM32_STD_SW_SDA_GPIO_PORT		GPIOB

// #define I2C_BUS_STM32_STD_SW_SCL_HIGH()			GPIO_SetBits(I2C_BUS_STM32_STD_SW_SCL_GPIO_PORT, I2C_BUS_STM32_STD_SW_SCL_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SCL_LOW()			GPIO_ResetBits(I2C_BUS_STM32_STD_SW_SCL_GPIO_PORT, I2C_BUS_STM32_STD_SW_SCL_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SCL_READ()			GPIO_ReadInputDataBit(I2C_BUS_STM32_STD_SW_SCL_GPIO_PORT, I2C_BUS_STM32_STD_SW_SCL_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SCL_WRITE(X)		GPIO_WriteBit(I2C_BUS_STM32_STD_SW_SCL_GPIO_PORT, I2C_BUS_STM32_STD_SW_SCL_GPIO_PIN, X ? Bit_SET : Bit_RESET)

// #define I2C_BUS_STM32_STD_SW_SDA_HIGH()			GPIO_SetBits(I2C_BUS_STM32_STD_SW_SDA_GPIO_PORT, I2C_BUS_STM32_STD_SW_SDA_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SDA_LOW()			GPIO_ResetBits(I2C_BUS_STM32_STD_SW_SDA_GPIO_PORT, I2C_BUS_STM32_STD_SW_SDA_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SDA_READ()			GPIO_ReadInputDataBit(I2C_BUS_STM32_STD_SW_SDA_GPIO_PORT, I2C_BUS_STM32_STD_SW_SDA_GPIO_PIN)
// #define I2C_BUS_STM32_STD_SW_SDA_WRITE(X)		GPIO_WriteBit(I2C_BUS_STM32_STD_SW_SDA_GPIO_PORT, I2C_BUS_STM32_STD_SW_SDA_GPIO_PIN, X ? Bit_SET : Bit_RESET)
