#include "spi_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define SCK_GPIO_PORT		GPIOB
#define SCK_GPIO_PIN		GPIO_Pin_8
#define SCK_GPIO_CLK		RCC_APB2Periph_GPIOB

#define MOSI_GPIO_PORT		GPIOB
#define MOSI_GPIO_PIN		GPIO_Pin_9
#define MOSI_GPIO_CLK		RCC_APB2Periph_GPIOB

#define MISO_GPIO_PORT		GPIOB
#define MISO_GPIO_PIN		GPIO_Pin_7
#define MISO_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SCK_HIGH()			GPIO_SetBits(SCK_GPIO_PORT, SCK_GPIO_PIN)
#define SCK_LOW()			GPIO_ResetBits(SCK_GPIO_PORT, SCK_GPIO_PIN)
#define SCK_READ()			GPIO_ReadInputDataBit(SCK_GPIO_PORT, SCK_GPIO_PIN)
#define SCK_WRITE(X)		GPIO_WriteBit(SCK_GPIO_PORT, SCK_GPIO_PIN, X ? Bit_SET : Bit_RESET)

#define MOSI_HIGH()			GPIO_SetBits(MOSI_GPIO_PORT, MOSI_GPIO_PIN)
#define MOSI_LOW()			GPIO_ResetBits(MOSI_GPIO_PORT, MOSI_GPIO_PIN)
#define MOSI_READ()			GPIO_ReadInputDataBit(MOSI_GPIO_PORT, MOSI_GPIO_PIN)
#define MOSI_WRITE(X)		GPIO_WriteBit(MOSI_GPIO_PORT, MOSI_GPIO_PIN, X ? Bit_SET : Bit_RESET)

#define MISO_READ()			GPIO_ReadInputDataBit(MISO_GPIO_PORT, MISO_GPIO_PIN)

static spi_bus_status_t Bus_Init(void *user_data) {
	if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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

    // 初始化状态：
	uint8_t cpol = (cfg->mode >> 1) & 0x01;
    sck_write(cfg, cpol);
	mosi_write(cfg, 0);
	
	return SPI_BUS_OK;
}

static spi_bus_status_t Bus_SwitchByte(void *user_data, uint8_t tx, uint8_t *rx) {
	if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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
    if (user_data == NULL || len == 0) { return SPI_BUS_ERR_INVALID_PARAM; }
	if (tx == NULL && rx == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
    
    for (uint16_t i = 0; i < len; i++) {
        uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
        uint8_t rx_byte;
        spi_bus_status_t ret = Bus_SwitchByte(user_data, tx_byte, &rx_byte);
        if (ret != SPI_BUS_OK) { return ret; }
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
	if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
	spi_bus_stm32_std_lib_sw_bus_config_t *cfg = (spi_bus_stm32_std_lib_sw_bus_config_t *)user_data;
	cfg->mode = mode;
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
    if (handle == NULL || cfg == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
	handle->user_data = cfg;
	handle->ops = &Bus_Ops;
	return handle->ops->init(handle->user_data);
}

#define CS_GPIO_PORT		GPIOB
#define CS_GPIO_PIN			GPIO_Pin_8
#define CS_GPIO_CLK			RCC_APB2Periph_GPIOB

#define CS_HIGH()			GPIO_SetBits(CS_GPIO_PORT, CS_GPIO_PIN)
#define CS_LOW()			GPIO_ResetBits(CS_GPIO_PORT, CS_GPIO_PIN)
#define CS_READ()			GPIO_ReadInputDataBit(CS_GPIO_PORT, CS_GPIO_PIN)
#define CS_WRITE(X)			GPIO_WriteBit(CS_GPIO_PORT, CS_GPIO_PIN, X ? Bit_SET : Bit_RESET)

static inline void cs_high(spi_bus_stm32_std_lib_sw_cs_config_t *cfg) {
	GPIO_SetBits(cfg->cs_gpio_port, cfg->cs_gpio_pin);
}

static inline void cs_low(spi_bus_stm32_std_lib_sw_cs_config_t *cfg) {
	GPIO_ResetBits(cfg->cs_gpio_port, cfg->cs_gpio_pin);
}

static inline uint8_t cs_read(spi_bus_stm32_std_lib_sw_cs_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->cs_gpio_port, cfg->cs_gpio_pin);
}

static inline void cs_write(spi_bus_stm32_std_lib_sw_cs_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, val ? Bit_SET : Bit_RESET);
}

static spi_bus_status_t Cs_Init(void *user_data) {
	if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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
	
	return SPI_BUS_OK;
}

static spi_bus_status_t Cs_Low(void *user_data) {
    if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
    spi_bus_stm32_std_lib_sw_cs_config_t *cfg = (spi_bus_stm32_std_lib_sw_cs_config_t *)user_data;
    GPIO_WriteBit(cfg->cs_gpio_port, cfg->cs_gpio_pin, Bit_RESET);
    return SPI_BUS_OK;
}

static spi_bus_status_t Cs_High(void *user_data) {
    if (user_data == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
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
	if (handle == NULL || cfg == NULL) { return SPI_BUS_ERR_INVALID_PARAM; }
	handle->user_data = (void*)cfg;
	handle->ops = &Cs_Ops;
	return handle->ops->init(handle->user_data);
}
