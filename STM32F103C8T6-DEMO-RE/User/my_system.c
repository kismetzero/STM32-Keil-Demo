#include "my_system.h"
#include <stdio.h>

#define LOG_TAG "system"
#include "elog.h"

void elog_config_init(void) {
	/* initialize EasyLogger */
	ElogErrCode ret = elog_init();
	if (ret != ELOG_NO_ERR) { while(1) { __NOP(); } }
	/* set EasyLogger log format */
	elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
	elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
	elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
	/* start EasyLogger */
	elog_start();
}

i2c_bus_handle_t i2c_handle;
i2c_bus_stm32_std_sw_config_t swi2c_cfg;
void swi2c_config_init(void) {
	swi2c_cfg.scl_gpio_clk	= RCC_APB2Periph_GPIOB;
	swi2c_cfg.scl_gpio_pin	= GPIO_Pin_8;
	swi2c_cfg.scl_gpio_port	= GPIOB;
	
	swi2c_cfg.sda_gpio_clk	= RCC_APB2Periph_GPIOB;
	swi2c_cfg.sda_gpio_pin	= GPIO_Pin_9;
	swi2c_cfg.sda_gpio_port	= GPIOB;
	
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_create_handle(&i2c_handle, &swi2c_cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("swi2c_config_init: Fail (Code: %d)", ret);
		return;
	}
	log_d("swi2c_config_init: Success!");
}

spi_bus_handle_t spi_bus_handle;
spi_bus_stm32_std_sw_config_t swspi_bus_cfg;
void swspi_bus_config_init(void) {
	swspi_bus_cfg.sck_gpio_clk		= RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.sck_gpio_pin		= GPIO_Pin_5;
	swspi_bus_cfg.sck_gpio_port		= GPIOA;
	
	swspi_bus_cfg.mosi_gpio_clk		= RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.mosi_gpio_pin		= GPIO_Pin_7;
	swspi_bus_cfg.mosi_gpio_port	= GPIOA;
	
	swspi_bus_cfg.miso_gpio_clk		= RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.miso_gpio_pin		= GPIO_Pin_6;
	swspi_bus_cfg.miso_gpio_port	= GPIOA;
	
	swspi_bus_cfg.mode				= 0;
	
	spi_bus_status_t ret = spi_bus_stm32_std_sw_create_handle(&spi_bus_handle, &swspi_bus_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("swspi_bus_config_init: Fail (Code: %d)", ret);
		return;
	}
	log_d("swspi_bus_config_init: Success!");
}

spi_bus_stm32_std_hw_config_t hwspi_bus_cfg;
void hwspi_bus_config_init(void) {
	hwspi_bus_cfg.spi_periph	= SPI1;
	hwspi_bus_cfg.spi_clk		= RCC_APB2Periph_SPI1;
	hwspi_bus_cfg.spi_gpio_clk	= RCC_APB2Periph_GPIOA;
	hwspi_bus_cfg.sck_gpio_pin	= GPIO_Pin_5;
	hwspi_bus_cfg.mosi_gpio_pin	= GPIO_Pin_7;
	hwspi_bus_cfg.miso_gpio_pin	= GPIO_Pin_6;
	hwspi_bus_cfg.spi_gpio_port	= GPIOA;

	hwspi_bus_cfg.mode			= 0;
	
	spi_bus_status_t ret = spi_bus_stm32_std_hw_create_handle(&spi_bus_handle, &hwspi_bus_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("hwspi_bus_config_init: Fail (Code: %d)", ret);
		return;
	}
	log_d("hwspi_bus_config_init: Success!");
}

char system_time[9];
void system_sync_time(void) {
	DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
	snprintf(system_time, sizeof(system_time), "%02d:%02d:%02d", ds3231_datetime.hour, ds3231_datetime.min, ds3231_datetime.sec);
}

DS3231_Handle_t ds3231_handle;
DS3231_DateTime_t ds3231_datetime;
void ds3231_config_init(void) {
	DS3231_Status_t ret = DS3231_Init(&ds3231_handle, &i2c_handle, 0);
	if (ret != DS3231_STATUS_OK) {
		log_e("ds3231_config_init: Fail (Code: %d)", ret);
		return;
	}
	DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
	system_sync_time();
	log_d("ds3231_config_init: Success!");
}

AHT20_Handle_t aht20_handle;
void aht20_config_init(void) {
	AHT20_Status_t ret = AHT20_Init(&aht20_handle, &i2c_handle, 0);
	if (ret != AHT20_STATUS_OK) {
		log_e("aht20_config_init: Fail (Code: %d)", ret);
		return;
	}
	log_d("aht20_config_init: Success!");
}

SHT40_Handle_t sht40_handle;
void sht40_config_init(void) {
	SHT40_Status_t ret = SHT40_Init(&sht40_handle, &i2c_handle, 0, 0);
	if (ret != SHT40_STATUS_OK) {
		log_e("sht40_config_init: Fail (Code: %d)", ret);
		return;
	}
	log_d("sht40_config_init: Success!");
}

spi_cs_handle_t w25qx_cs_handle;
spi_bus_stm32_std_cs_config_t w25qx_cs_cfg;
spi_dev_handle_t w25qx_spi_handle;
W25QX_Handle_t w25qx_handle;
void w25qx_config_init(void) {
	w25qx_cs_cfg.cs_gpio_clk	= RCC_APB2Periph_GPIOA;
	w25qx_cs_cfg.cs_gpio_pin	= GPIO_Pin_4;
	w25qx_cs_cfg.cs_gpio_port	= GPIOA;
	
	spi_bus_status_t ret = spi_bus_stm32_std_cs_create_handle(&w25qx_cs_handle, &w25qx_cs_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("w25qx_config_init: Fail (Code: %d)", ret);
		return;
	}
	
	w25qx_spi_handle.bus	= &spi_bus_handle;
	w25qx_spi_handle.cs		= &w25qx_cs_handle;
	
	W25QX_Status_t w25qx_ret = W25QX_Init(&w25qx_handle, &w25qx_spi_handle);
	if (w25qx_ret != W25QX_STATUS_OK) {
		log_e("w25qx_config_init: Fail (Code: %d)", w25qx_ret);
		return;
	}
	log_d("w25qx_config_init: Success!");
}


void system_init(void) {
	delay_init();
	elog_config_init();
	swi2c_config_init();
//	swspi_bus_config_init();
	hwspi_bus_config_init();
	w25qx_config_init();
	aht20_config_init();
	sht40_config_init();
	ds3231_config_init();
}

void system_proc(void) {
	
}
