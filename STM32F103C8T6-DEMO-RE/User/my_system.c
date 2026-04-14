#include "my_system.h"
#include <stdio.h>

#define LOG_TAG "system"
#include "elog.h"

i2c_bus_handle_t i2c_handle;
spi_bus_handle_t spi_bus_handle;

DS3231_Handle_t ds3231_handle;
DS3231_DateTime_t ds3231_datetime;

AHT20_Handle_t aht20_handle;
SHT40_Handle_t sht40_handle;

W25QX_Handle_t w25qx_handle;

static i2c_bus_stm32_std_sw_config_t swi2c_cfg = {
	.scl_gpio_clk	= RCC_APB2Periph_GPIOB,
	.scl_gpio_pin	= GPIO_Pin_8,
	.scl_gpio_port	= GPIOB,
	
	.sda_gpio_clk	= RCC_APB2Periph_GPIOB,
	.sda_gpio_pin	= GPIO_Pin_9,
	.sda_gpio_port	= GPIOB,
};

static spi_bus_stm32_std_sw_config_t swspi_bus_cfg = {
	.sck_gpio_clk	= RCC_APB2Periph_GPIOA,
	.sck_gpio_pin	= GPIO_Pin_5,
	.sck_gpio_port	= GPIOA,
	
	.mosi_gpio_clk	= RCC_APB2Periph_GPIOA,
	.mosi_gpio_pin	= GPIO_Pin_7,
	.mosi_gpio_port	= GPIOA,
	
	.miso_gpio_clk	= RCC_APB2Periph_GPIOA,
	.miso_gpio_pin	= GPIO_Pin_6,
	.miso_gpio_port	= GPIOA,
	
	.mode			= 0,
};

static spi_bus_stm32_std_hw_config_t hwspi_bus_cfg = {
	.spi_periph		= SPI1,
	.spi_clk		= RCC_APB2Periph_SPI1,
	.spi_gpio_clk	= RCC_APB2Periph_GPIOA,
	.sck_gpio_pin	= GPIO_Pin_5,
	.mosi_gpio_pin	= GPIO_Pin_7,
	.miso_gpio_pin	= GPIO_Pin_6,
	.spi_gpio_port	= GPIOA,

	.mode			= 0,
};

static spi_cs_handle_t w25qx_cs_handle;
static spi_bus_stm32_std_cs_config_t w25qx_cs_cfg = {
	.cs_gpio_clk	= RCC_APB2Periph_GPIOA,
	.cs_gpio_pin	= GPIO_Pin_4,
	.cs_gpio_port	= GPIOA,
};
static spi_dev_handle_t w25qx_spi_handle = {
	.bus	= &spi_bus_handle,
	.cs		= &w25qx_cs_handle,
};

//void my_elog_assert_hook(const char* expr, const char* func, size_t line){
//	log_a("Assert failed: (%s) in %s:%d", expr, func, line);
//	
//	while (1) {
//        __NOP(); 
//    }
//}

static void elog_config_init(void) {
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
//	elog_assert_set_hook(my_elog_assert_hook);
	/* start EasyLogger */
	elog_start();
}

static uint8_t swi2c_config_init(void) {
	i2c_bus_status_t ret = i2c_bus_stm32_std_sw_create_handle(&i2c_handle, &swi2c_cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("swi2c_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	return 0;
}

static uint8_t swspi_config_init(void) {
	spi_bus_status_t ret = spi_bus_stm32_std_sw_create_handle(&spi_bus_handle, &swspi_bus_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("swspi_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	return 0;
}

static uint8_t hwspi_config_init(void) {
	spi_bus_status_t ret = spi_bus_stm32_std_hw_create_handle(&spi_bus_handle, &hwspi_bus_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("hwspi_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	return 0;
}

static uint8_t ds3231_config_init(void) {
	DS3231_Status_t ret = DS3231_Init(&ds3231_handle, &i2c_handle, 0);
	if (ret != DS3231_STATUS_OK) {
		log_e("ds3231_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
	system_sync_time();
	return 0;
}

static uint8_t aht20_config_init(void) {
	AHT20_Status_t ret = AHT20_Init(&aht20_handle, &i2c_handle, 0);
	if (ret != AHT20_STATUS_OK) {
		log_e("aht20_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	return 0;
}

static uint8_t sht40_config_init(void) {
	SHT40_Status_t ret = SHT40_Init(&sht40_handle, &i2c_handle, 0, 0);
	if (ret != SHT40_STATUS_OK) {
		log_e("sht40_config_init: Fail! (Code: %d)", ret);
		return -1;
	}
	return 0;
}

static void w25qx_config_init(void) {
	spi_bus_status_t ret = spi_bus_stm32_std_cs_create_handle(&w25qx_cs_handle, &w25qx_cs_cfg);
	if (ret != SPI_BUS_STATUS_OK) {
		log_e("w25qx_config_init: Fail! @ cs (Code: %d)", ret);
		return;
	}
	
	W25QX_Status_t w25qx_ret = W25QX_Init(&w25qx_handle, &w25qx_spi_handle);
	if (w25qx_ret != W25QX_STATUS_OK) {
		log_e("w25qx_config_init: Fail! @ init (Code: %d)", w25qx_ret);
		return;
	}
	log_d("w25qx_config_init: Success!");
}

char system_time[9] = "00:00:00";
void system_sync_time(void) {
	DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
	snprintf(system_time, sizeof(system_time), "%02d:%02d:%02d", ds3231_datetime.hour, ds3231_datetime.min, ds3231_datetime.sec);
}

void system_init(void) {
	delay_init();
	elog_config_init();
	
	swi2c_config_init();
//	swspi_config_init();
	hwspi_config_init();
	
	ds3231_config_init();
	aht20_config_init();
	sht40_config_init();

	w25qx_config_init();
}

void system_proc(void) {
	
}
