#include "spi_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define LOG_TAG "spi_impl"
#include "elog.h"

#define SPI_BUS_FAST 0

__STATIC_INLINE void __spi_delay(void) {
	delay_us(5);
}

__STATIC_INLINE uint8_t __sck_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->sck_gpio_port, cfg->sck_gpio_pin);
}

__STATIC_INLINE void __sck_write(spi_bus_stm32_std_lib_sw_bus_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sck_gpio_port, cfg->sck_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE uint8_t __mosi_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->mosi_gpio_port, cfg->mosi_gpio_pin);
}

__STATIC_INLINE void __mosi_write(spi_bus_stm32_std_lib_sw_bus_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->mosi_gpio_port, cfg->mosi_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE uint8_t __miso_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->miso_gpio_port, cfg->miso_gpio_pin);
}

__STATIC_INLINE spi_bus_status_t __switch_byte(spi_bus_stm32_std_lib_sw_bus_config_t *cfg, uint8_t tx, uint8_t *rx) {
	uint8_t cpol = (cfg->mode >> 1) & 0x01;
	uint8_t cpha = cfg->mode & 0x01;
	uint8_t res = 0x00;
	for (uint8_t i = 0; i < 8; i++) {
		if (tx & (0x80 >> i)) { __mosi_write(cfg, 1); }
		else { __mosi_write(cfg, 0); }
		__spi_delay(); 
		
		if (cpha == 0) {
			__sck_write(cfg, !cpol);
			__spi_delay();
			if (__miso_read(cfg)) { res |= (0x80 >> i); }
			__sck_write(cfg, cpol);
		} else {
			__sck_write(cfg, !cpol);
			__spi_delay();
			__sck_write(cfg, cpol);
			__spi_delay();
			if (__miso_read(cfg)) { res |= (0x80 >> i); }
		}
		__spi_delay();
	}
	log_d("__switch_byte: tx=0x%02X, rx=0x%02X", tx, res);
	if (rx != NULL) { *rx = res; }
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t __bus_init(spi_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("__bus_init: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__bus_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	RCC_APB2PeriphClockCmd(cfg->sck_gpio_clk | cfg->mosi_gpio_clk | cfg->miso_gpio_clk, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCK 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->sck_gpio_pin;
	GPIO_Init(cfg->sck_gpio_port, &GPIO_InitStructure);
	// MOSI 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->mosi_gpio_pin;
	GPIO_Init(cfg->mosi_gpio_port, &GPIO_InitStructure);
	// MISO 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = cfg->miso_gpio_pin;
	GPIO_Init(cfg->miso_gpio_port, &GPIO_InitStructure);
	// 计算并打印模式
	uint8_t mode = cfg->mode;
	if (mode > 0x03) {
		log_w("__bus_init: Warning! Invalid mode %d detected! Auto-correcting to %d.", (uint8_t)mode, (uint8_t)(mode & 0x03));
		mode &= 0x03;
	}
	cfg->mode = mode;
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("__bus_init: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	// 初始化状态：
	__sck_write(cfg, cpol);
	__mosi_write(cfg, 0);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t __bus_switch_byte(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__bus_switch_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__bus_switch_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	return __switch_byte(cfg, tx, rx);
}

static spi_bus_status_t __bus_write_byte(spi_bus_handle_t *handle, uint8_t byte) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__bus_write_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__bus_write_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	return __switch_byte(cfg, byte, NULL);
}

static spi_bus_status_t __bus_read_byte(spi_bus_handle_t *handle, uint8_t *byte) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__bus_read_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__bus_read_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	return __switch_byte(cfg, 0xFF, byte);
}

static spi_bus_status_t __bus_switch_bytes(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__bus_switch_bytes: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__bus_switch_bytes: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("__bus_switch_bytes: Fail! Both tx and rx are NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("__bus_switch_bytes: Fail! len == 0");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	for (uint16_t i = 0; i < len; i++) {
		uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
		uint8_t rx_byte;
		spi_bus_status_t ret = __switch_byte(cfg, tx_byte, &rx_byte);
		if (ret != SPI_BUS_STATUS_OK) {
			log_e("__bus_switch_bytes: Fail! @ index %d", i);
			return ret;
		}
		if (rx != NULL) { rx[i] = rx_byte; }
	}
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t __bus_write_bytes(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len) {
	return __bus_switch_bytes(handle, data, NULL, len);
}

static spi_bus_status_t __bus_read_bytes(spi_bus_handle_t *handle, uint8_t *data, uint16_t len) {
	return __bus_switch_bytes(handle, NULL, data, len);	
}

static spi_bus_status_t __bus_set_mode(spi_bus_handle_t *handle, spi_bus_mode_t mode) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__bus_set_mode: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__bus_set_mode: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)handle->user_data;
	if (mode > 0x03) {
		log_w("__bus_set_mode: Warning! Invalid mode %d detected! Auto-correcting to %d.", (uint8_t)mode, (uint8_t)(mode & 0x03));
		mode &= 0x03;
	}
	cfg->mode = mode;
	// 计算并打印模式
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	__sck_write(cfg, cpol);
	log_i("__bus_set_mode: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_ops_t __bus_ops = {
	.init = __bus_init,
	.switch_byte = __bus_switch_byte,
	.read_byte = __bus_read_byte,
	.write_byte = __bus_write_byte,
	.switch_bytes = __bus_switch_bytes,
	.read_bytes = __bus_read_bytes,
	.write_bytes = __bus_write_bytes,
	.set_mode = __bus_set_mode
};

spi_bus_status_t spi_bus_stm32_std_lib_sw_create_bus_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &__bus_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_lib_sw_create_bus_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

static spi_bus_status_t __cs_init(spi_cs_handle_t *handle) {
	if (handle == NULL) {
		log_e("__cs_init: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("__cs_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)handle->user_data;
	RCC_APB2PeriphClockCmd(cfg->cs_gpio_clk, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// CS 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->cs_gpio_pin;
	GPIO_Init(cfg->cs_gpio_port, &GPIO_InitStructure);
	// 默认拉高 (释放设备)
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_SET);
	log_i("__cs_init: Success! cs_pin=%d", cfg->cs_gpio_pin);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t __cs_high(spi_cs_handle_t *handle) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__cs_high: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__cs_high: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)handle->user_data;
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_SET);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t __cs_low(spi_cs_handle_t *handle) {
#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("__cs_low: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("__cs_low: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)handle->user_data;
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_RESET);
	return SPI_BUS_STATUS_OK;
}

static spi_cs_ops_t __cs_ops = {
	.init = __cs_init,
	.high = __cs_high,
	.low = __cs_low
};

spi_bus_status_t spi_bus_stm32_std_lib_sw_create_cs_handle(spi_cs_handle_t *handle, spi_bus_stm32_std_lib_sw_cs_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = (void*)cfg;
	handle->ops = &__cs_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_lib_sw_create_cs_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

//static spi_bus_status_t HW_Init(void *user_data) {
//	if (user_data == NULL) {
//		log_e("HW_Init: Fail: user_data == NULL");
//		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
//	}
//	spi_bus_stm32_std_lib_hw_bus_config_t *cfg = (spi_bus_stm32_std_lib_hw_bus_config_t *)user_data;
//	RCC_APB2PeriphClockCmd(cfg->cs_gpio_clk, ENABLE);
//	GPIO_InitTypeDef GPIO_InitStructure;
//	//GPIO_StructInit(&GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	// CS 引脚初始化
//	GPIO_InitStructure.GPIO_Pin = cfg->cs_gpio_pin;
//	GPIO_Init(cfg->cs_gpio_port, &GPIO_InitStructure);
//	// 默认拉高 (释放设备)
//	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_SET);
//	log_i("__cs_init: Success cs_pin=%d", cfg->cs_gpio_pin);
//	return SPI_BUS_STATUS_OK;
//}
