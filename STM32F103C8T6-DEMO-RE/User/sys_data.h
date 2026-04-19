#ifndef __SYS_DATA_H
#define __SYS_DATA_H

#include "sys_core.h"

#ifdef __cplusplus
	extern "C" {
#endif

#if SYS_USE_I2C_BUS_STM32_STD
	extern i2c_bus_handle_t i2c_handle;
#endif /* SYS_USE_I2C_BUS_STM32_STD */

#if SYS_USE_SPI_BUS_STM32_STD
	extern spi_bus_handle_t spi_bus_handle;
#endif /* SYS_USE_SPI_BUS_STM32_STD */

#if SYS_USE_DS3231
	extern DS3231_Handle_t ds3231_handle;
	extern DS3231_DateTime_t ds3231_datetime;
#endif /* SYS_USE_DS3231 */

#if SYS_USE_W25QX
	extern W25QX_Handle_t w25qx_handle;
#endif /* SYS_USE_W25QX */

#if SYS_USE_AHT20
	extern AHT20_Handle_t aht20_handle;
#endif /* SYS_USE_AHT20 */

#if SYS_USE_SHT40
	extern SHT40_Handle_t sht40_handle;
#endif /* SYS_USE_SHT40 */

#if SYS_USE_MAX7219
#endif /* SYS_USE_MAX7219 */

#if SYS_USE_SSD1306
#endif /* SYS_USE_SSD1306 */

#if SYS_USE_RTC
	extern char sys_time_str[9];
#endif /* SYS_USE_RTC */

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_DATA_H */