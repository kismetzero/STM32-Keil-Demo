#ifndef __SYS_DATA_H
#define __SYS_DATA_H

#include "sys_inc.h"

#ifdef __cplusplus
	extern "C" {
#endif

extern sys_init_status_t sys_init_status;

#if (SYS_EN_I2C_BUS_STM32_STD == 1) && (defined(SYS_USE_I2C_BUS_STM32_STD) || defined(SYS_USE_ALL_EN))
	extern i2c_bus_handle_t i2c_handle;
#endif /* SYS_I2C_BUS_STM32_STD */

#if (SYS_EN_SPI_BUS_STM32_STD == 1) && (defined(SYS_USE_SPI_BUS_STM32_STD) || defined(SYS_USE_ALL_EN))
	extern spi_bus_handle_t spi_bus_handle;
#endif /* SYS_SPI_BUS_STM32_STD */

#if (SYS_EN_DS3231 == 1) && (defined(SYS_USE_DS3231) || defined(SYS_USE_ALL_EN))
	extern DS3231_Handle_t ds3231_handle;
	extern DS3231_DateTime_t ds3231_datetime;
#endif /* SYS_DS3231 */

#if (SYS_EN_W25QX == 1) && (defined(SYS_USE_W25QX) || defined(SYS_USE_ALL_EN))
	extern W25QX_Handle_t w25qx_handle;
#endif /* SYS_W25QX */

#if (SYS_EN_AHT20 == 1) && (defined(SYS_USE_AHT20) || defined(SYS_USE_ALL_EN))
	extern AHT20_Handle_t aht20_handle;
#endif /* SYS_AHT20 */

#if (SYS_EN_SHT40 == 1) && (defined(SYS_USE_SHT40) || defined(SYS_USE_ALL_EN))
	extern SHT40_Handle_t sht40_handle;
#endif /* SYS_SHT40 */

#if (SYS_EN_MAX7219 == 1) && (defined(SYS_USE_MAX7219) || defined(SYS_USE_ALL_EN))
	extern MAX7219_Handle_t max7219_handle;
#endif /* SYS_MAX7219 */

#if (SYS_EN_SSD1306 == 1) && (defined(SYS_USE_SSD1306) || defined(SYS_USE_ALL_EN))
	extern SSD1306_Handle_t ssd1306_handle;
#endif /* SYS_SSD1306 */

#if (SYS_EN_RTC == 1) && (defined(SYS_USE_RTC) || defined(SYS_USE_ALL_EN))
	extern char sys_time_str[9];
#endif /* SYS_RTC */

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_DATA_H */