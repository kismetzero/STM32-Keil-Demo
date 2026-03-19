#include "spi_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define LOG_TAG "spi_impl"
#include "elog.h"

static inline void SWSPI_Delay(void) {
}

static inline uint8_t sck_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->sck_gpio_port, cfg->sck_gpio_pin);
}

static inline void sck_write(spi_bus_stm32_std_lib_sw_bus_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sck_gpio_port, cfg->sck_gpio_pin, val ? Bit_SET : Bit_RESET);
}

static inline uint8_t mosi_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->mosi_gpio_port, cfg->mosi_gpio_pin);
}

static inline void mosi_write(spi_bus_stm32_std_lib_sw_bus_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->mosi_gpio_port, cfg->mosi_gpio_pin, val ? Bit_SET : Bit_RESET);
}

static inline uint8_t miso_read(spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->miso_gpio_port, cfg->miso_gpio_pin);
}

static spi_bus_status_t Bus_Init(void *user_data) {
	if (user_data == NULL) {
		log_e("Bus_Init: Fail user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)user_data;
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
		log_w("Bus_Init: Invalid mode %d detected! Auto-correcting to %d.", (uint8_t)mode, (uint8_t)(mode & 0x03));
		mode &= 0x03;
	}
	cfg->mode = mode;
    uint8_t cpol = (mode >> 1) & 0x01;
    uint8_t cpha = mode & 0x01;
	log_i("Bus_Init: Success SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	// 初始化状态：
    sck_write(cfg, cpol);
	mosi_write(cfg, 0);
	return SPI_BUS_OK;
}

static spi_bus_status_t Bus_SwitchByte(void *user_data, uint8_t tx, uint8_t *rx) {
	if (user_data == NULL) {
		log_e("Bus_SwitchByte: Fail user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)user_data;
	uint8_t cpol = (cfg->mode >> 1) & 0x01;
    uint8_t cpha = cfg->mode & 0x01;
	uint8_t res = 0x00;
	for (uint8_t i = 0; i < 8; i++) {
		if (tx & (0x80 >> i)) { mosi_write(cfg, 1); }
		else { mosi_write(cfg, 0); }
        SWSPI_Delay(); 
		
		if (cpha == 0) {
			sck_write(cfg, !cpol);
			SWSPI_Delay();
			if (miso_read(cfg)) { res |= (0x80 >> i); }
			sck_write(cfg, cpol);
		} else {
			sck_write(cfg, !cpol);
			SWSPI_Delay();
			sck_write(cfg, cpol);
			SWSPI_Delay();
			if (miso_read(cfg)) { res |= (0x80 >> i); }
		}
		SWSPI_Delay();
	}
	log_d("Bus_SwitchByte: tx=%d, rx=%d", tx, res);
    if (rx != NULL) { *rx = res; }
    return SPI_BUS_OK;
}

static spi_bus_status_t Bus_WriteByte(void *user_data, uint8_t byte) {
    return Bus_SwitchByte(user_data, byte, NULL);
}

static spi_bus_status_t Bus_ReadByte(void *user_data, uint8_t *byte) {
	return Bus_SwitchByte(user_data, 0xFF, byte);
}

static spi_bus_status_t Bus_SwitchBytes(void *user_data, const uint8_t *tx, uint8_t *rx, uint16_t len) {
    if (user_data == NULL) {
		log_e("Bus_SwitchBytes: Fail user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (tx == NULL && rx == NULL) {
		log_e("Bus_SwitchBytes: Fail Both tx and rx are NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (len == 0) {
		log_e("Bus_SwitchBytes: Fail len == 0");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
    for (uint16_t i = 0; i < len; i++) {
        uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
        uint8_t rx_byte;
        spi_bus_status_t ret = Bus_SwitchByte(user_data, tx_byte, &rx_byte);
        if (ret != SPI_BUS_OK) {
			log_e("Bus_SwitchBytes: Fail @ index %d", i);
			return ret;
		}
        if (rx != NULL) { rx[i] = rx_byte; }
    }
    return SPI_BUS_OK;
}

static spi_bus_status_t Bus_WriteBytes(void *user_data, const uint8_t *data, uint16_t len) {
    return Bus_SwitchBytes(user_data, data, NULL, len);
}

static spi_bus_status_t Bus_ReadBytes(void *user_data, uint8_t *data, uint16_t len) {
    return Bus_SwitchBytes(user_data, NULL, data, len);	
}

static spi_bus_status_t Bus_SetMode(void *user_data, spi_bus_mode_t mode) {
	if (user_data == NULL) {
		log_e("Bus_SetMode: user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)user_data;
	if (mode > 0x03) {
		log_w("Bus_SetMode: Invalid mode %d detected! Auto-correcting to %d.", (uint8_t)mode, (uint8_t)(mode & 0x03));
		mode &= 0x03;
	}
	cfg->mode = mode;
	// 计算并打印模式
    uint8_t cpol = (mode >> 1) & 0x01;
    uint8_t cpha = mode & 0x01;
	sck_write(cfg, cpol);
	log_i("Bus_Init: SPI Mode=%d (CPOL=%d, CPHA=%d)", mode, cpol, cpha);
	return SPI_BUS_OK;
}

static spi_bus_ops_t Bus_Ops = {
	.init = Bus_Init,
	.switch_byte = Bus_SwitchByte,
	.read_byte = Bus_ReadByte,
    .write_byte = Bus_WriteByte,
    .switch_bytes = Bus_SwitchBytes,
	.read_bytes = Bus_ReadBytes,
	.write_bytes = Bus_WriteBytes,
	.set_mode = Bus_SetMode
};

spi_bus_status_t spi_bus_stm32_std_lib_sw_create_bus_handle(spi_bus_handle_t *handle, spi_bus_stm32_std_lib_sw_bus_config_t *cfg) {
    if (handle == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail handle == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail cfg == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	handle->user_data = cfg;
	handle->ops = &Bus_Ops;
	spi_bus_status_t ret = handle->ops->init(handle->user_data);
	if (ret == SPI_BUS_OK) {
		log_i("spi_bus_stm32_std_lib_sw_create_bus_handle: Init Handle Success");
	} else {
		log_e("spi_bus_stm32_std_lib_sw_create_bus_handle: Fail Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}

static spi_bus_status_t Cs_Init(void *user_data) {
	if (user_data == NULL) {
		log_e("Cs_Init: Fail: user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)user_data;
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
	log_i("Cs_Init: Success cs_pin=%d", cfg->cs_gpio_pin);
	return SPI_BUS_OK;
}

static spi_bus_status_t Cs_Low(void *user_data) {
    if (user_data == NULL) {
		log_e("Cs_Low: Fail user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
    spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)user_data;
    GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_RESET);
    return SPI_BUS_OK;
}

static spi_bus_status_t Cs_High(void *user_data) {
    if (user_data == NULL) {
		log_e("Cs_High: Fail user_data == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
    spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)user_data;
    GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_SET);
    return SPI_BUS_OK;
}

static spi_cs_ops_t Cs_Ops = {
    .init = Cs_Init,
    .low = Cs_Low,
    .high = Cs_High
};

spi_bus_status_t spi_bus_stm32_std_lib_sw_create_cs_handle(spi_cs_handle_t *handle, spi_bus_stm32_std_lib_sw_cs_config_t *cfg) {
	if (handle == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail handle == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	if (cfg == NULL) {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail cfg == NULL");
		return SPI_BUS_ERR_INVALID_PARAM;
	}
	handle->user_data = (void*)cfg;
	handle->ops = &Cs_Ops;
	spi_bus_status_t ret = handle->ops->init(handle->user_data);
	if (ret == SPI_BUS_OK) {
		log_i("spi_bus_stm32_std_lib_sw_create_cs_handle: Init Handle Success");
	} else {
		log_e("spi_bus_stm32_std_lib_sw_create_cs_handle: Fail Init Handle Fail (Code: %d)", ret);
	}
	return ret;
}
