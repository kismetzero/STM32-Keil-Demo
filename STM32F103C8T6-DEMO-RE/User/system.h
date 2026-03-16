#include "stm32f10x.h"
#include "delay.h"
#include "elog.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std_lib_sw.h"
#include "spi_bus.h"
#include "spi_bus_stm32_std_lib_sw.h"

void system_init();
void elog_config_init();

i2c_bus_handle_t swi2c_handle;
i2c_bus_stm32_std_lib_sw_config_t swi2c_cfg;
void i2c_config_init();

spi_bus_handle_t swspi_bus_handle;
spi_bus_stm32_std_lib_sw_bus_config_t swspi_bus_cfg;
void spi_bus_config_init();

void system_init() {
	delay_init();
	elog_config_init();
	i2c_config_init();
	spi_bus_config_init();
}

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

void i2c_config_init() {
	swi2c_cfg.scl_gpio_clk	=	RCC_APB2Periph_GPIOB;
	swi2c_cfg.scl_gpio_pin	=	GPIO_Pin_8;
	swi2c_cfg.scl_gpio_port	=	GPIOB;
	
	swi2c_cfg.sda_gpio_clk	=	RCC_APB2Periph_GPIOB;
	swi2c_cfg.sda_gpio_pin	=	GPIO_Pin_9;
	swi2c_cfg.sda_gpio_port	=	GPIOB;
	
	i2c_bus_status_t ret = i2c_bus_stm32_std_lib_sw_create_handle(&swi2c_handle, &swi2c_cfg);
	if (ret != I2C_BUS_OK) {
		log_e("i2c_config_init: erro");
	}
}

void spi_bus_config_init() {
	swspi_bus_cfg.miso_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.miso_gpio_pin		=	GPIO_Pin_4;
	swspi_bus_cfg.miso_gpio_port	=	GPIOA;
	
	swspi_bus_cfg.mosi_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.mosi_gpio_pin		=	GPIO_Pin_6;
	swspi_bus_cfg.mosi_gpio_port	=	GPIOA;
	
	swspi_bus_cfg.sck_gpio_clk		=	RCC_APB2Periph_GPIOA;
	swspi_bus_cfg.sck_gpio_pin		=	GPIO_Pin_5;
	swspi_bus_cfg.sck_gpio_port		=	GPIOA;
	
	spi_bus_status_t ret = spi_bus_stm32_std_lib_sw_create_bus_handle(&swspi_bus_handle, &swspi_bus_cfg);
	if (ret != SPI_BUS_OK) {
		log_e("spi_bus_config_init: erro");
	}
}

//void w25qx_test() {
//	spi_dev_handle_t w25qx;
//	spi_bus_handle_t swspi_bus_handle;
//	spi_bus_stm32_std_lib_sw_bus_config_t swspi_bus_cfg;
//	spi_cs_handle_t swspi_cs_handle;
//	spi_bus_stm32_std_lib_sw_cs_config_t swspi_cs_cfg;
//	spi_test(&swspi_bus_cfg, &swspi_cs_cfg);
//	spi_bus_status_t ret;
//	ret = spi_bus_stm32_std_lib_sw_create_bus_handle(&swspi_bus_handle, &swspi_bus_cfg);
//	Serial_Printf("spi_bus_stm32_std_lib_sw_create_bus_handle: %d \n", ret);
//	ret = spi_bus_stm32_std_lib_sw_create_cs_handle(&swspi_cs_handle, &swspi_cs_cfg);
//	Serial_Printf("spi_bus_stm32_std_lib_sw_create_cs_handle: %d \n", ret);
//	w25qx.bus = &swspi_bus_handle;
//	w25qx.cs = &swspi_cs_handle;
//	uint8_t cmd[] = {0x9F};
//	uint8_t ret_data[8] = {0};
//	
//	ret = spi_cs_low(&w25qx);
//	Serial_Printf("spi_cs_low: %d \n", ret);
//	ret = spi_master_transmit(&w25qx, cmd, 1);
//	Serial_Printf("spi_master_transmit: %d \n", ret);
//	
//	ret = spi_master_receive(&w25qx, ret_data, 3);
//	Serial_Printf("spi_master_receive: %d \n", ret);
//	ret = spi_cs_high(&w25qx);
//	Serial_Printf("spi_cs_high: %d \n", ret);
//	
//	uint16_t did = (ret_data[1] << 8) | ret_data[2];
//	Serial_Printf("w25q64 mid: %d \n", ret_data[0]);
//	Serial_Printf("w25q64 did: %d \n", did);
//}

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

