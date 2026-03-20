#include "stm32f10x.h"
#include "delay.h"
#include "elog.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std_lib_sw.h"
#include "spi_bus.h"
#include "spi_bus_stm32_std_lib_sw.h"
#include "AHT20.h"

void elog_config_init() {
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

i2c_bus_handle_t swi2c_handle;
i2c_bus_stm32_std_lib_sw_config_t swi2c_cfg;
void i2c_config_init() {
	swi2c_cfg.scl_gpio_clk	=	RCC_APB2Periph_GPIOB;
	swi2c_cfg.scl_gpio_pin	=	GPIO_Pin_8;
	swi2c_cfg.scl_gpio_port	=	GPIOB;
	
	swi2c_cfg.sda_gpio_clk	=	RCC_APB2Periph_GPIOB;
	swi2c_cfg.sda_gpio_pin	=	GPIO_Pin_9;
	swi2c_cfg.sda_gpio_port	=	GPIOB;
	
	i2c_bus_status_t ret = i2c_bus_stm32_std_lib_sw_create_handle(&swi2c_handle, &swi2c_cfg);
	if (ret != I2C_BUS_STATUS_OK) {
		log_e("i2c_config_init: Fail (Code: %d)", ret);
	}
}

AHT20_Handle_t aht20_handle;
void aht20_config_init() {
	aht20_handle.hi2c		=	&swi2c_handle;
	aht20_handle.i2c_addr	=	0;
	AHT20_Init(&aht20_handle);
}

spi_bus_handle_t swspi_bus_handle;
spi_bus_stm32_std_lib_sw_bus_config_t swspi_bus_cfg;
void spi_bus_config_init() {
	swspi_bus_cfg.miso_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.miso_gpio_pin		=	GPIO_Pin_6;
	swspi_bus_cfg.miso_gpio_port	=	GPIOA;
	
	swspi_bus_cfg.mosi_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.mosi_gpio_pin		=	GPIO_Pin_7;
	swspi_bus_cfg.mosi_gpio_port	=	GPIOA;
	
	swspi_bus_cfg.sck_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.sck_gpio_pin		=	GPIO_Pin_5;
	swspi_bus_cfg.sck_gpio_port		=	GPIOA;
	
	swspi_bus_cfg.mode				=	0;
	
	spi_bus_status_t ret = spi_bus_stm32_std_lib_sw_create_bus_handle(&swspi_bus_handle, &swspi_bus_cfg);
	if (ret != SPI_BUS_OK) {
		log_e("spi_bus_config_init: Fail (Code: %d)", ret);
	}
}

spi_cs_handle_t w25qx_cs_handle;
spi_bus_stm32_std_lib_sw_cs_config_t w25qx_cs_cfg;
spi_dev_handle_t w25qx_dev_handle;
void w25qx_config_init() {
	w25qx_cs_cfg.cs_gpio_clk	=	RCC_APB2Periph_GPIOA;
	w25qx_cs_cfg.cs_gpio_pin	=	GPIO_Pin_4;
	w25qx_cs_cfg.cs_gpio_port	=	GPIOA;
	
	spi_bus_status_t ret = spi_bus_stm32_std_lib_sw_create_cs_handle(&w25qx_cs_handle, &w25qx_cs_cfg);
	if (ret != SPI_BUS_OK) {
		log_e("w25qx_config_init: Fail (Code: %d)", ret);
	}
	
	w25qx_dev_handle.bus	=	&swspi_bus_handle;
	w25qx_dev_handle.cs		=	&w25qx_cs_handle;
}
void w25qx_test() {
	spi_bus_status_t ret;
	uint8_t cmd[] = {0x9F};
	uint8_t ret_data[8] = {0};
	
	ret = spi_cs_low(&w25qx_dev_handle);
	if (ret != SPI_BUS_OK) {
		log_e("w25qx_test: spi_cs_low: Fail (Code: %d)", ret);
		return;
	}
	ret = spi_master_transmit(&w25qx_dev_handle, cmd, 1);
	if (ret != SPI_BUS_OK) {
		log_e("w25qx_test: spi_master_transmit: Fail (Code: %d)", ret);
		return;
	}
	
	ret = spi_master_receive(&w25qx_dev_handle, ret_data, 3);
	if (ret != SPI_BUS_OK) {
		log_e("w25qx_test: spi_master_receive: Fail (Code: %d)", ret);
		return;
	}
	ret = spi_cs_high(&w25qx_dev_handle);
	if (ret != SPI_BUS_OK) {
		log_e("w25qx_test: spi_cs_high: Fail (Code: %d)", ret);
		return;
	}
	
	uint16_t did = (ret_data[1] << 8) | ret_data[2];
	log_i("w25qx_test: mid: %d \n", ret_data[0]);
	log_i("w25qx_test: did: %d \n", did);
}

void system_init() {
	delay_init();
	elog_config_init();
	i2c_config_init();
	spi_bus_config_init();
	w25qx_config_init();
	aht20_config_init();
}


//void aht20_test() {
//	AHT20_Handle_t sensor;
//	AHT20_Status_t ret;
//	sensor.hi2c = &swi2c_handle;
//	ret = AHT20_Init(&sensor);
//	log_d("sensor-init: %d \n", ret);
//	log_i("sensor-temp: %f \n", sensor.temperature);
//	log_i("sensor-humi: %f \n", sensor.humidity);
//
//	ret = AHT20_Measure(&sensor);
//	log_d("sensor-measure: %d \n", ret);
//	log_i("sensor-temp: %f \n", sensor.temperature);
//	log_i("sensor-humi: %f \n", sensor.humidity);
//}

//void sht40_test() {
//	SHT40_Handle_t sensor;
//	SHT40_Status_t ret;
//	sensor.hi2c = &swi2c_handle;
//	ret = SHT40_Init(&sensor);
//	log_d("sensor-init: %d \n", ret);
//	log_i("sensor-temp: %f \n", sensor.temperature);
//	log_i("sensor-humi: %f \n", sensor.humidity);
//
//	ret = SHT40_Measure(&sensor);
//	log_d("sensor-measure: %d \n", ret);
//	log_i("sensor-temp: %f \n", sensor.temperature);
//	log_i("sensor-humi: %f \n", sensor.humidity);
//}

