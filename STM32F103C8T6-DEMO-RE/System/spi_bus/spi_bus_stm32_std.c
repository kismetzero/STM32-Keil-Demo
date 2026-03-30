#include "spi_bus_stm32_std.h"
#include "delay.h"

#define LOG_TAG "spi_impl"
#include "elog.h"

#define SPI_BUS_FAST 0

//=====================================================================================================
// SPI SW BUS OOP

__STATIC_INLINE void spi_bus_stm32_std_sw_delay(void) {
	delay_us(5);
}

__STATIC_INLINE void spi_bus_stm32_std_sw_sck_write(spi_bus_stm32_std_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sck_gpio_port, cfg->sck_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE void spi_bus_stm32_std_sw_mosi_write(spi_bus_stm32_std_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->mosi_gpio_port, cfg->mosi_gpio_pin, val ? Bit_SET : Bit_RESET);
}

__STATIC_INLINE uint8_t spi_bus_stm32_std_sw_miso_read(spi_bus_stm32_std_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->miso_gpio_port, cfg->miso_gpio_pin);
}

__STATIC_INLINE spi_bus_status_t spi_bus_stm32_std_sw_switch(spi_bus_stm32_std_sw_config_t *cfg, uint8_t tx, uint8_t *rx) {
	uint8_t cpol = (cfg->mode >> 1) & 0x01;
	uint8_t cpha = cfg->mode & 0x01;
	uint8_t res = 0x00;
	for (uint8_t i = 0; i < 8; i++) {
		if (tx & (0x80 >> i)) { spi_bus_stm32_std_sw_mosi_write(cfg, 1); }
		else { spi_bus_stm32_std_sw_mosi_write(cfg, 0); }
		spi_bus_stm32_std_sw_delay(); 
		
		if (cpha == 0) {
			spi_bus_stm32_std_sw_sck_write(cfg, !cpol);
			spi_bus_stm32_std_sw_delay();
			if (spi_bus_stm32_std_sw_miso_read(cfg)) { res |= (0x80 >> i); }
			spi_bus_stm32_std_sw_sck_write(cfg, cpol);
		} else {
			spi_bus_stm32_std_sw_sck_write(cfg, !cpol);
			spi_bus_stm32_std_sw_delay();
			spi_bus_stm32_std_sw_sck_write(cfg, cpol);
			spi_bus_stm32_std_sw_delay();
			if (spi_bus_stm32_std_sw_miso_read(cfg)) { res |= (0x80 >> i); }
		}
		spi_bus_stm32_std_sw_delay();
	}
	log_d("spi_bus_stm32_std_sw_switch: tx=0x%02X, rx=0x%02X", tx, res);
	if (rx != NULL) { *rx = res; }
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_sw_init(spi_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_init: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	
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
		mode &= 0x03;
		log_w("spi_bus_stm32_std_sw_init: Warning! Invalid mode %d detected! Auto-correcting to %d.", cfg->mode, mode);
		cfg->mode = mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_sw_init: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	
	// 初始化状态：
	spi_bus_stm32_std_sw_sck_write(cfg, cpol);
	spi_bus_stm32_std_sw_mosi_write(cfg, 0);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_sw_switch_byte(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	return spi_bus_stm32_std_sw_switch(cfg, tx, rx);
}

static spi_bus_status_t spi_bus_stm32_std_sw_read_byte(spi_bus_handle_t *handle, uint8_t *byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_read_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_read_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	return spi_bus_stm32_std_sw_switch(cfg, 0xFF, byte);
}

static spi_bus_status_t spi_bus_stm32_std_sw_write_byte(spi_bus_handle_t *handle, uint8_t byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_write_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_write_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	return spi_bus_stm32_std_sw_switch(cfg, byte, NULL);
}

static spi_bus_status_t spi_bus_stm32_std_sw_switch_bytes(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! Both tx and rx are NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! len == 0");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	for (uint16_t i = 0; i < len; i++) {
		uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
		uint8_t rx_byte;
		spi_bus_status_t ret = spi_bus_stm32_std_sw_switch(cfg, tx_byte, &rx_byte);
		if (ret != SPI_BUS_STATUS_OK) {
			log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! @ index %d", i);
			return ret;
		}
		if (rx != NULL) { rx[i] = rx_byte; }
	}
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_sw_read_bytes(spi_bus_handle_t *handle, uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_sw_switch_bytes(handle, NULL, data, len);	
}

static spi_bus_status_t spi_bus_stm32_std_sw_write_bytes(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_sw_switch_bytes(handle, data, NULL, len);
}

static spi_bus_status_t spi_bus_stm32_std_sw_set_mode(spi_bus_handle_t *handle, spi_bus_mode_t mode) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_set_mode: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_sw_set_mode: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_sw_config_t *cfg = (spi_bus_stm32_std_sw_config_t *)handle->user_data;
	// 计算并打印模式
	cfg->mode = mode;
	if (mode > 0x03) {
		cfg->mode = mode & 0x03;
		log_w("spi_bus_stm32_std_sw_set_mode: Warning! Invalid mode %d detected! Auto-correcting to %d.", mode, cfg->mode);
		mode = cfg->mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	spi_bus_stm32_std_sw_sck_write(cfg, cpol);
	log_i("spi_bus_stm32_std_sw_set_mode: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_ops_t spi_bus_stm32_std_sw_ops = {
	.init = spi_bus_stm32_std_sw_init,
	.switch_byte = spi_bus_stm32_std_sw_switch_byte,
	.read_byte = spi_bus_stm32_std_sw_read_byte,
	.write_byte = spi_bus_stm32_std_sw_write_byte,
	.switch_bytes = spi_bus_stm32_std_sw_switch_bytes,
	.read_bytes = spi_bus_stm32_std_sw_read_bytes,
	.write_bytes = spi_bus_stm32_std_sw_write_bytes,
	.set_mode = spi_bus_stm32_std_sw_set_mode
};

spi_bus_status_t spi_bus_stm32_std_sw_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_sw_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_bus_create_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_sw_bus_create_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &spi_bus_stm32_std_sw_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_sw_bus_create_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_sw_bus_create_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

//=====================================================================================================


//=====================================================================================================
// SPI CS OOP

static spi_bus_status_t spi_bus_stm32_std_cs_init(spi_cs_handle_t *handle) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_cs_init: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_cs_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_cs_config_t *cfg = (spi_bus_stm32_std_cs_config_t *)handle->user_data;
	
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
	log_i("spi_bus_stm32_std_cs_init: Success! cs_pin=%d", cfg->cs_gpio_pin);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_cs_high(spi_cs_handle_t *handle) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_cs_high: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_cs_high: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_cs_config_t *cfg = (spi_bus_stm32_std_cs_config_t *)handle->user_data;
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_SET);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_cs_low(spi_cs_handle_t *handle) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_cs_low: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_cs_low: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_cs_config_t *cfg = (spi_bus_stm32_std_cs_config_t *)handle->user_data;
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_RESET);
	return SPI_BUS_STATUS_OK;
}

static spi_cs_ops_t __cs_ops = {
	.init = spi_bus_stm32_std_cs_init,
	.high = spi_bus_stm32_std_cs_high,
	.low = spi_bus_stm32_std_cs_low
};

spi_bus_status_t spi_bus_stm32_std_cs_create_handle(spi_cs_handle_t *handle, spi_bus_stm32_std_cs_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_cs_create_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_cs_create_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = (void*)cfg;
	handle->ops = &__cs_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_cs_create_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_cs_create_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

//=====================================================================================================


//=====================================================================================================
// SPI HW BUS OOP

static spi_bus_status_t spi_bus_stm32_std_hw_init(spi_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_init: Fail!: handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	
	RCC_APB2PeriphClockCmd(cfg->spi_gpio_clk, ENABLE);
	if (cfg->spi_periph == SPI1) {
		RCC_APB2PeriphClockCmd(cfg->spi_clk, ENABLE);
	} else {
		RCC_APB1PeriphClockCmd(cfg->spi_clk, ENABLE);
	}
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCK 和 MOSI 引脚初始化
	GPIO_InitStructure.GPIO_Pin = cfg->sck_gpio_pin | cfg->mosi_gpio_pin;
	GPIO_Init(cfg->spi_gpio_port, &GPIO_InitStructure);
	// MISO 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = cfg->miso_gpio_pin;
	GPIO_Init(cfg->spi_gpio_port, &GPIO_InitStructure);
	
	// 计算并打印模式
	uint8_t mode = cfg->mode;
	if (mode > 0x03) {
		mode &= 0x03;
		log_w("spi_bus_stm32_std_hw_init: Warning! Invalid mode %d detected! Auto-correcting to %d.", cfg->mode, mode);
		cfg->mode = mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_hw_init: Info! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	
	SPI_InitTypeDef SPI_InitStructure;
	//SPI_StructInit(&SPI_InitStructure);
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							// 主从模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		// 工作模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						// 数据帧大小
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						// 高位先行
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;	// 分频系数
	SPI_InitStructure.SPI_CPOL = cpol ? SPI_CPOL_High : SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = cpha ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 0x0007;							// CRC校验参数，手册默认0x0007
	SPI_Init(cfg->spi_periph, &SPI_InitStructure);
	
	SPI_Cmd(cfg->spi_periph, ENABLE);
	
	return SPI_BUS_STATUS_OK;
}

__STATIC_INLINE spi_bus_status_t spi_bus_stm32_std_hw_switch(spi_bus_stm32_std_hw_config_t *cfg, uint8_t tx, uint8_t *rx) {
	uint8_t res = 0x00;
	while (SPI_I2S_GetFlagStatus(cfg->spi_periph, SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(cfg->spi_periph, tx);
	while (SPI_I2S_GetFlagStatus(cfg->spi_periph, SPI_I2S_FLAG_RXNE) != SET);
	res = SPI_I2S_ReceiveData(cfg->spi_periph);
	log_d("spi_bus_stm32_std_hw_switch: tx=0x%02X, rx=0x%02X", tx, res);
	if (rx != NULL) { *rx = res; }
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_hw_switch_byte(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_switch_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_switch_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	return spi_bus_stm32_std_hw_switch(cfg, tx, rx);
}

static spi_bus_status_t spi_bus_stm32_std_hw_read_byte(spi_bus_handle_t *handle, uint8_t *byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_read_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_read_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	return spi_bus_stm32_std_hw_switch(cfg, 0xFF, byte);
}

static spi_bus_status_t spi_bus_stm32_std_hw_write_byte(spi_bus_handle_t *handle, uint8_t byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_write_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_write_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	return spi_bus_stm32_std_hw_switch(cfg, byte, NULL);
}

static spi_bus_status_t spi_bus_stm32_std_hw_switch_bytes(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_sw_switch_bytes: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_switch_bytes: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("spi_bus_stm32_std_hw_switch_bytes: Fail! Both tx and rx are NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_bus_stm32_std_hw_switch_bytes: Fail! len == 0");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	for (uint16_t i = 0; i < len; i++) {
		uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
		uint8_t rx_byte;
		spi_bus_status_t ret = spi_bus_stm32_std_hw_switch(cfg, tx_byte, &rx_byte);
		if (ret != SPI_BUS_STATUS_OK) {
			log_e("spi_bus_stm32_std_hw_switch_bytes: Fail! @ index %d", i);
			return ret;
		}
		if (rx != NULL) { rx[i] = rx_byte; }
	}
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_hw_read_bytes(spi_bus_handle_t *handle, uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_hw_switch_bytes(handle, NULL, data, len);	
}

static spi_bus_status_t spi_bus_stm32_std_hw_write_bytes(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_hw_switch_bytes(handle, data, NULL, len);
}

static spi_bus_status_t spi_bus_stm32_std_hw_set_mode(spi_bus_handle_t *handle, spi_bus_mode_t mode) {
	// 硬件 SPI 不方便切换模式
	return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_set_mode: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_hw_set_mode: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_hw_config_t *cfg = (spi_bus_stm32_std_hw_config_t *)handle->user_data;
	// 计算并打印模式
	cfg->mode = mode;
	if (mode > 0x03) {
		cfg->mode = mode & 0x03;
		log_w("spi_bus_stm32_std_hw_set_mode: Warning! Invalid mode %d detected! Auto-correcting to %d.", mode, cfg->mode);
		mode = cfg->mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_hw_set_mode: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_ops_t spi_bus_stm32_std_hw_ops = {
	.init = spi_bus_stm32_std_hw_init,
	.switch_byte = spi_bus_stm32_std_hw_switch_byte,
	.read_byte = spi_bus_stm32_std_hw_read_byte,
	.write_byte = spi_bus_stm32_std_hw_write_byte,
	.switch_bytes = spi_bus_stm32_std_hw_switch_bytes,
	.read_bytes = spi_bus_stm32_std_hw_read_bytes,
	.write_bytes = spi_bus_stm32_std_hw_write_bytes,
	.set_mode = spi_bus_stm32_std_hw_set_mode
};

spi_bus_status_t spi_bus_stm32_std_hw_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_hw_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_hw_create_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_hw_create_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &spi_bus_stm32_std_hw_ops;
	if (cfg->spi_periph == SPI1) {
		cfg->spi_clk		= RCC_APB2Periph_SPI1;
		cfg->spi_gpio_clk	= RCC_APB2Periph_GPIOA;
		cfg->sck_gpio_pin	= GPIO_Pin_5;
		cfg->mosi_gpio_pin	= GPIO_Pin_7;
		cfg->miso_gpio_pin	= GPIO_Pin_6;
		cfg->spi_gpio_port	= GPIOA;
	}
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_hw_create_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_hw_create_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

//=====================================================================================================


//=====================================================================================================
// SPI SW BUS MACRO

#define SPI_BUS_STM32_STD_SW_SCK_GPIO_CLK		RCC_APB2Periph_GPIOA
#define SPI_BUS_STM32_STD_SW_SCK_GPIO_PIN		GPIO_Pin_5
#define SPI_BUS_STM32_STD_SW_SCK_GPIO_PORT		GPIOA

#define SPI_BUS_STM32_STD_SW_MOSI_GPIO_CLK		RCC_APB2Periph_GPIOA
#define SPI_BUS_STM32_STD_SW_MOSI_GPIO_PIN		GPIO_Pin_7
#define SPI_BUS_STM32_STD_SW_MOSI_GPIO_PORT		GPIOA

#define SPI_BUS_STM32_STD_SW_MISO_GPIO_CLK		RCC_APB2Periph_GPIOA
#define SPI_BUS_STM32_STD_SW_MISO_GPIO_PIN		GPIO_Pin_6
#define SPI_BUS_STM32_STD_SW_MISO_GPIO_PORT		GPIOA

#define SPI_BUS_STM32_STD_SW_SCK_WRITE(X)		GPIO_WriteBit(SPI_BUS_STM32_STD_SW_SCK_GPIO_PORT, SPI_BUS_STM32_STD_SW_SCK_GPIO_PIN, X ? Bit_SET : Bit_RESET)
#define SPI_BUS_STM32_STD_SW_MOSI_WRITE(X)		GPIO_WriteBit(SPI_BUS_STM32_STD_SW_MOSI_GPIO_PORT, SPI_BUS_STM32_STD_SW_MOSI_GPIO_PIN, X ? Bit_SET : Bit_RESET)
#define SPI_BUS_STM32_STD_SW_MISO_READ()		GPIO_ReadInputDataBit(SPI_BUS_STM32_STD_SW_MISO_GPIO_PORT, SPI_BUS_STM32_STD_SW_MISO_GPIO_PIN)

__STATIC_INLINE spi_bus_status_t spi_bus_stm32_std_SW_switch(uint8_t mode, uint8_t tx, uint8_t *rx) {
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	uint8_t res = 0x00;
	for (uint8_t i = 0; i < 8; i++) {
		if (tx & (0x80 >> i)) { SPI_BUS_STM32_STD_SW_MOSI_WRITE(1); }
		else { SPI_BUS_STM32_STD_SW_MOSI_WRITE(0); }
		spi_bus_stm32_std_sw_delay(); 
		if (cpha == 0) {
			SPI_BUS_STM32_STD_SW_SCK_WRITE(!cpol);
			spi_bus_stm32_std_sw_delay();
			if (SPI_BUS_STM32_STD_SW_MISO_READ()) { res |= (0x80 >> i); }
			SPI_BUS_STM32_STD_SW_SCK_WRITE(cpol);
		} else {
			SPI_BUS_STM32_STD_SW_SCK_WRITE(!cpol);
			spi_bus_stm32_std_sw_delay();
			SPI_BUS_STM32_STD_SW_SCK_WRITE(cpol);
			spi_bus_stm32_std_sw_delay();
			if (SPI_BUS_STM32_STD_SW_MISO_READ()) { res |= (0x80 >> i); }
		}
		spi_bus_stm32_std_sw_delay();
	}
	log_d("spi_bus_stm32_std_SW_switch: tx=0x%02X, rx=0x%02X", tx, res);
	if (rx != NULL) { *rx = res; }
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_SW_init(spi_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_init: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	
	RCC_APB2PeriphClockCmd(SPI_BUS_STM32_STD_SW_SCK_GPIO_CLK | SPI_BUS_STM32_STD_SW_MOSI_GPIO_CLK | SPI_BUS_STM32_STD_SW_MISO_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCK 引脚初始化
	GPIO_InitStructure.GPIO_Pin = SPI_BUS_STM32_STD_SW_SCK_GPIO_PIN;
	GPIO_Init(SPI_BUS_STM32_STD_SW_SCK_GPIO_PORT, &GPIO_InitStructure);
	// MOSI 引脚初始化
	GPIO_InitStructure.GPIO_Pin = SPI_BUS_STM32_STD_SW_MOSI_GPIO_PIN;
	GPIO_Init(SPI_BUS_STM32_STD_SW_MOSI_GPIO_PORT, &GPIO_InitStructure);
	// MISO 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SPI_BUS_STM32_STD_SW_MISO_GPIO_PIN;
	GPIO_Init(SPI_BUS_STM32_STD_SW_MISO_GPIO_PORT, &GPIO_InitStructure);
	
	// 计算并打印模式
	uint8_t mode = cfg->mode;
	if (mode > 0x03) {
		mode &= 0x03;
		log_w("spi_bus_stm32_std_SW_init: Warning! Invalid mode %d detected! Auto-correcting to %d.", cfg->mode, mode);
		cfg->mode = mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_SW_init: Info! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	
	// 初始化状态：
	SPI_BUS_STM32_STD_SW_SCK_WRITE(cpol);
	SPI_BUS_STM32_STD_SW_MOSI_WRITE(0);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_SW_switch_byte(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_switch_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_switch_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	return spi_bus_stm32_std_SW_switch(cfg->mode, tx, rx);
}

static spi_bus_status_t spi_bus_stm32_std_SW_read_byte(spi_bus_handle_t *handle, uint8_t *byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_read_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_read_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	return spi_bus_stm32_std_SW_switch(cfg->mode, 0xFF, byte);
}

static spi_bus_status_t spi_bus_stm32_std_SW_write_byte(spi_bus_handle_t *handle, uint8_t byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_write_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_write_byte: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	return spi_bus_stm32_std_SW_switch(cfg->mode, byte, NULL);
}

static spi_bus_status_t spi_bus_stm32_std_SW_switch_bytes(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_switch_bytes: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_switch_bytes: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("spi_bus_stm32_std_SW_switch_bytes: Fail! Both tx and rx are NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_bus_stm32_std_SW_switch_bytes: Fail! len == 0");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	for (uint16_t i = 0; i < len; i++) {
		uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
		uint8_t rx_byte;
		spi_bus_status_t ret = spi_bus_stm32_std_SW_switch(cfg->mode, tx_byte, &rx_byte);
		if (ret != SPI_BUS_STATUS_OK) {
			log_e("spi_bus_stm32_std_SW_switch_bytes: Fail! @ index %d", i);
			return ret;
		}
		if (rx != NULL) { rx[i] = rx_byte; }
	}
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_SW_read_bytes(spi_bus_handle_t *handle, uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_SW_switch_bytes(handle, NULL, data, len);	
}

static spi_bus_status_t spi_bus_stm32_std_SW_write_bytes(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_SW_switch_bytes(handle, data, NULL, len);
}

static spi_bus_status_t spi_bus_stm32_std_SW_set_mode(spi_bus_handle_t *handle, spi_bus_mode_t mode) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_set_mode: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_SW_set_mode: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_SW_config_t *cfg = (spi_bus_stm32_std_SW_config_t *)handle->user_data;
	// 计算并打印模式
	cfg->mode = mode;
	if (mode > 0x03) {
		cfg->mode = mode & 0x03;
		log_w("spi_bus_stm32_std_SW_set_mode: Warning! Invalid mode %d detected! Auto-correcting to %d.", mode, cfg->mode);
		mode = cfg->mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	SPI_BUS_STM32_STD_SW_SCK_WRITE(cpol);
	log_i("spi_bus_stm32_std_SW_set_mode: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_ops_t spi_bus_stm32_std_SW_ops = {
	.init = spi_bus_stm32_std_SW_init,
	.switch_byte = spi_bus_stm32_std_SW_switch_byte,
	.read_byte = spi_bus_stm32_std_SW_read_byte,
	.write_byte = spi_bus_stm32_std_SW_write_byte,
	.switch_bytes = spi_bus_stm32_std_SW_switch_bytes,
	.read_bytes = spi_bus_stm32_std_SW_read_bytes,
	.write_bytes = spi_bus_stm32_std_SW_write_bytes,
	.set_mode = spi_bus_stm32_std_SW_set_mode
};

spi_bus_status_t spi_bus_stm32_std_SW_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_SW_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_SW_create_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_SW_create_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &spi_bus_stm32_std_SW_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_SW_create_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_SW_create_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

//=====================================================================================================


//=====================================================================================================
// SPI HW BUS MACRO

#define SPI_BUS_STM32_STD_HW_SPI_CLK			RCC_APB2Periph_SPI1
#define SPI_BUS_STM32_STD_HW_SPI_PERIPH			SPI1
#define SPI_BUS_STM32_STD_HW_GPIO_CLK			RCC_APB2Periph_GPIOA
#define SPI_BUS_STM32_STD_HW_SCK_GPIO_PIN		GPIO_Pin_5
#define SPI_BUS_STM32_STD_HW_MOSI_GPIO_PIN		GPIO_Pin_7
#define SPI_BUS_STM32_STD_HW_MISO_GPIO_PIN		GPIO_Pin_6
#define SPI_BUS_STM32_STD_HW_GPIO_PORT			GPIOA

static spi_bus_status_t spi_bus_stm32_std_HW_init(spi_bus_handle_t *handle) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_init: Fail!: handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_HW_init: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_HW_config_t *cfg = (spi_bus_stm32_std_HW_config_t *)handle->user_data;
	
	RCC_APB2PeriphClockCmd(SPI_BUS_STM32_STD_HW_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SPI_BUS_STM32_STD_HW_SPI_CLK, ENABLE);
//	if (SPI_BUS_STM32_STD_HW_SPIX == SPI1) {
//		RCC_APB2PeriphClockCmd(SPI_BUS_STM32_STD_HW_SPI_CLK, ENABLE);
//	} else {
//		RCC_APB1PeriphClockCmd(SPI_BUS_STM32_STD_HW_SPI_CLK, ENABLE);
//	}
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCK 和 MOSI 引脚初始化
	GPIO_InitStructure.GPIO_Pin = SPI_BUS_STM32_STD_HW_SCK_GPIO_PIN | SPI_BUS_STM32_STD_HW_MOSI_GPIO_PIN;
	GPIO_Init(SPI_BUS_STM32_STD_HW_GPIO_PORT, &GPIO_InitStructure);
	// MISO 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SPI_BUS_STM32_STD_HW_MISO_GPIO_PIN;
	GPIO_Init(SPI_BUS_STM32_STD_HW_GPIO_PORT, &GPIO_InitStructure);
	
	// 计算并打印模式
	uint8_t mode = cfg->mode;
	if (mode > 0x03) {
		mode &= 0x03;
		log_w("spi_bus_stm32_std_hw_init: Warning! Invalid mode %d detected! Auto-correcting to %d.", cfg->mode, mode);
		cfg->mode = mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_hw_init: Info! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	
	SPI_InitTypeDef SPI_InitStructure;
	//SPI_StructInit(&SPI_InitStructure);
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							// 主从模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		// 工作模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						// 数据帧大小
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						// 高位先行
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;	// 分频系数
	SPI_InitStructure.SPI_CPOL = cpol ? SPI_CPOL_High : SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = cpha ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 0x0007;							// CRC校验参数，手册默认0x0007
	SPI_Init(SPI_BUS_STM32_STD_HW_SPI_PERIPH, &SPI_InitStructure);
	
	SPI_Cmd(SPI_BUS_STM32_STD_HW_SPI_PERIPH, ENABLE);
	
	return SPI_BUS_STATUS_OK;
}

__STATIC_INLINE spi_bus_status_t spi_bus_stm32_std_HW_switch(uint8_t tx, uint8_t *rx) {
	uint8_t res = 0x00;
	while (SPI_I2S_GetFlagStatus(SPI_BUS_STM32_STD_HW_SPI_PERIPH, SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(SPI_BUS_STM32_STD_HW_SPI_PERIPH, tx);
	while (SPI_I2S_GetFlagStatus(SPI_BUS_STM32_STD_HW_SPI_PERIPH, SPI_I2S_FLAG_RXNE) != SET);
	res = SPI_I2S_ReceiveData(SPI_BUS_STM32_STD_HW_SPI_PERIPH);
	log_d("spi_bus_stm32_std_HW_switch: tx=0x%02X, rx=0x%02X", tx, res);
	if (rx != NULL) { *rx = res; }
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_HW_switch_byte(spi_bus_handle_t *handle, uint8_t tx, uint8_t *rx) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_switch_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	return spi_bus_stm32_std_HW_switch(tx, rx);
}

static spi_bus_status_t spi_bus_stm32_std_HW_read_byte(spi_bus_handle_t *handle, uint8_t *byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_read_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	return spi_bus_stm32_std_HW_switch(0xFF, byte);
}

static spi_bus_status_t spi_bus_stm32_std_HW_write_byte(spi_bus_handle_t *handle, uint8_t byte) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_write_byte: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	return spi_bus_stm32_std_HW_switch(byte, NULL);
}

static spi_bus_status_t spi_bus_stm32_std_HW_switch_bytes(spi_bus_handle_t *handle, const uint8_t *tx, uint8_t *rx, uint16_t len) {
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_switch_bytes: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_HW_switch_bytes: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("spi_bus_stm32_std_HW_switch_bytes: Fail! Both tx and rx are NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("spi_bus_stm32_std_HW_switch_bytes: Fail! len == 0");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_HW_config_t *cfg = (spi_bus_stm32_std_HW_config_t *)handle->user_data;
	for (uint16_t i = 0; i < len; i++) {
		uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
		uint8_t rx_byte;
		spi_bus_status_t ret = spi_bus_stm32_std_HW_switch(tx_byte, &rx_byte);
		if (ret != SPI_BUS_STATUS_OK) {
			log_e("spi_bus_stm32_std_HW_switch_bytes: Fail! @ index %d", i);
			return ret;
		}
		if (rx != NULL) { rx[i] = rx_byte; }
	}
	return SPI_BUS_STATUS_OK;
}

static spi_bus_status_t spi_bus_stm32_std_HW_read_bytes(spi_bus_handle_t *handle, uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_HW_switch_bytes(handle, NULL, data, len);	
}

static spi_bus_status_t spi_bus_stm32_std_HW_write_bytes(spi_bus_handle_t *handle, const uint8_t *data, uint16_t len) {
	return spi_bus_stm32_std_HW_switch_bytes(handle, data, NULL, len);
}

static spi_bus_status_t spi_bus_stm32_std_HW_set_mode(spi_bus_handle_t *handle, spi_bus_mode_t mode) {
	// 硬件 SPI 不方便切换模式
	return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	#if SPI_BUS_FAST == 0
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_set_mode: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	#endif	/* SPI_BUS_FAST */
	if (handle->user_data == NULL) {
		log_e("spi_bus_stm32_std_HW_set_mode: Fail! user_data == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_HW_config_t *cfg = (spi_bus_stm32_std_HW_config_t *)handle->user_data;
	// 计算并打印模式
	cfg->mode = mode;
	if (mode > 0x03) {
		cfg->mode = mode & 0x03;
		log_w("spi_bus_stm32_std_HW_set_mode: Warning! Invalid mode %d detected! Auto-correcting to %d.", mode, cfg->mode);
		mode = cfg->mode;
	}
	uint8_t cpol = (mode >> 1) & 0x01;
	uint8_t cpha = mode & 0x01;
	log_i("spi_bus_stm32_std_HW_set_mode: Success! SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_STATUS_OK;
}

static spi_bus_ops_t spi_bus_stm32_std_HW_ops = {
	.init = spi_bus_stm32_std_HW_init,
	.switch_byte = spi_bus_stm32_std_HW_switch_byte,
	.read_byte = spi_bus_stm32_std_HW_read_byte,
	.write_byte = spi_bus_stm32_std_HW_write_byte,
	.switch_bytes = spi_bus_stm32_std_HW_switch_bytes,
	.read_bytes = spi_bus_stm32_std_HW_read_bytes,
	.write_bytes = spi_bus_stm32_std_HW_write_bytes,
	.set_mode = spi_bus_stm32_std_HW_set_mode
};

spi_bus_status_t spi_bus_stm32_std_HW_create_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_HW_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_HW_create_handle: Fail! handle == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_HW_create_handle: Fail! cfg == NULL");
		return SPI_BUS_STATUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &spi_bus_stm32_std_HW_ops;
	spi_bus_status_t ret = handle->ops->init(handle);
	if (ret == SPI_BUS_STATUS_OK) {
		log_i("spi_bus_stm32_std_HW_create_handle: Success! Init Handle");
	} else {
		log_e("spi_bus_stm32_std_HW_create_handle: Fail! Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}
