#include "sys_core.h"
#include "delay.h"

#include <stdio.h>

#define LOG_TAG "sys"
#include "elog.h"

#if SYS_USE_EASYLOGGER
//	void my_elog_assert_hook(const char* expr, const char* func, size_t line){
//		log_a("Assert failed: (%s) in %s:%d", expr, func, line);
//		
//		while (1) {
//			__NOP(); 
//		}
//	}
#endif /* SYS_USE_EASYLOGGER */

#if SYS_USE_I2C_BUS_STM32_STD
	i2c_bus_handle_t i2c_handle;
	static i2c_bus_stm32_std_sw_config_t swi2c_cfg = {
		.scl_gpio_clk	= RCC_APB2Periph_GPIOB,
		.scl_gpio_pin	= GPIO_Pin_8,
		.scl_gpio_port	= GPIOB,
		
		.sda_gpio_clk	= RCC_APB2Periph_GPIOB,
		.sda_gpio_pin	= GPIO_Pin_9,
		.sda_gpio_port	= GPIOB,
	};
#endif /* SYS_USE_I2C_BUS_STM32_STD */

#if SYS_USE_SPI_BUS_STM32_STD
	spi_bus_handle_t spi_bus_handle;
	#if SYS_USE_SPI_BUS_STM32_STD_HW
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
	#else  /* SYS_USE_SPI_BUS_STM32_STD_HW */
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
	#endif /* SYS_USE_SPI_BUS_STM32_STD_HW */
#endif /* SYS_USE_SPI_BUS_STM32_STD */

#if SYS_USE_DS3231
	DS3231_Handle_t ds3231_handle;
	DS3231_DateTime_t ds3231_datetime;
#endif /* SYS_USE_DS3231 */

#if SYS_USE_W25QX
	W25QX_Handle_t w25qx_handle;
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
#endif /* SYS_USE_W25QX */

#if SYS_USE_AHT20
	AHT20_Handle_t aht20_handle;
#endif /* SYS_USE_AHT20 */

#if SYS_USE_SHT40
	SHT40_Handle_t sht40_handle;
#endif /* SYS_USE_SHT40 */
	
#if SYS_USE_RTC
	char sys_time_str[9] = "00:00:00";
	void sys_sync_time(void) {
		#if SYS_USE_DS3231
			DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
			snprintf(sys_time_str, sizeof(sys_time_str), "%02d:%02d:%02d", ds3231_datetime.hour, ds3231_datetime.min, ds3231_datetime.sec);
		#endif /* SYS_USE_DS3231 */
	}
	
#endif /* SYS_USE_RTC */

void sys_core_init() {
	delay_init();
	uint8_t ret;
	#if SYS_USE_EASYLOGGER
		/* initialize EasyLogger */
		ret = elog_init();
		if (ret != ELOG_NO_ERR) { while(1) { __NOP(); } }
		/* set EasyLogger log format */
		elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
		elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
		elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
		elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
		elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
		elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
//		elog_assert_set_hook(my_elog_assert_hook);
		/* start EasyLogger */
		elog_start();
	#endif /* SYS_USE_EASYLOGGER */
	
	#if SYS_USE_I2C_BUS_STM32_STD
		ret = i2c_bus_stm32_std_sw_create_handle(&i2c_handle, &swi2c_cfg);
		if	(ret != I2C_BUS_STATUS_OK) {
			log_e("i2c init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_I2C_BUS_STM32_STD */

	#if SYS_USE_SPI_BUS_STM32_STD
		#if SYS_USE_SPI_BUS_STM32_STD_HW
			ret = spi_bus_stm32_std_hw_create_handle(&spi_bus_handle, &hwspi_bus_cfg);
		#else  /* SYS_USE_SPI_BUS_STM32_STD_HW */
			ret = spi_bus_stm32_std_hw_create_handle(&spi_bus_handle, &hwspi_bus_cfg);
		#endif /* SYS_USE_SPI_BUS_STM32_STD_HW */
		if	(ret != SPI_BUS_STATUS_OK) {
			log_e("spi bus init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_SPI_BUS_STM32_STD */

	#if SYS_USE_DS3231
		ret = DS3231_Init(&ds3231_handle, &i2c_handle, 0);
		if	(ret != DS3231_STATUS_OK) {
			log_e("DS3231 init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_DS3231 */

	#if SYS_USE_W25QX
		ret = spi_bus_stm32_std_cs_create_handle(&w25qx_cs_handle, &w25qx_cs_cfg);
		if	(ret != SPI_BUS_STATUS_OK) {
			log_e("W25QX cs init fail (code: %d)", ret);
		}
		ret = W25QX_Init(&w25qx_handle, &w25qx_spi_handle);
		if	(ret != W25QX_STATUS_OK) {
			log_e("W25QX init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_W25QX */

	#if SYS_USE_AHT20
		ret = AHT20_Init(&aht20_handle, &i2c_handle, 0);
		if	(ret != AHT20_STATUS_OK) {
			log_e("AHT20 init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_AHT20 */

	#if SYS_USE_SHT40
		ret = SHT40_Init(&sht40_handle, &i2c_handle, 0, 0);
		if	(ret != SHT40_STATUS_OK) {
			log_e("SHT40 init fail (code: %d)", ret);
		}
	#endif /* SYS_USE_SHT40 */
}
