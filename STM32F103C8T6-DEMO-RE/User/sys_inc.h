#ifndef __SYS_INC_H
#define __SYS_INC_H

#include "sys_config.h"
#include "stm32f10x.h"

#if (SYS_EN_FREERTOS == 1) && (defined(SYS_USE_FREERTOS) || defined(SYS_USE_ALL_EN))
	#include "FreeRTOS.h"
	#include "semphr.h"
	#include "task.h"
#endif /* SYS_FREERTOS */

#if (SYS_EN_SERIAL == 1) && (defined(SYS_USE_SERIAL) || defined(SYS_USE_ALL_EN))
	#include "Serial.h"
#endif /* SYS_SERIAL */

#if (SYS_EN_I2C_BUS_STM32_STD == 1) && (defined(SYS_USE_I2C_BUS_STM32_STD) || defined(SYS_USE_ALL_EN))
	#include "i2c_bus_stm32_std.h"
#endif /* SYS_I2C_BUS_STM32_STD */

#if (SYS_EN_SPI_BUS_STM32_STD == 1) && (defined(SYS_USE_SPI_BUS_STM32_STD) || defined(SYS_USE_ALL_EN))
	#include "spi_bus_stm32_std.h"
#endif /* SYS_SPI_BUS_STM32_STD */

#if (SYS_EN_DS3231 == 1) && (defined(SYS_USE_DS3231) || defined(SYS_USE_ALL_EN))
	#include "DS3231.h"
#endif /* SYS_DS3231 */

#if (SYS_EN_W25QX == 1) && (defined(SYS_USE_W25QX) || defined(SYS_USE_ALL_EN))
	#include "W25QX.h"
#endif /* SYS_W25QX */

#if (SYS_EN_AHT20 == 1) && (defined(SYS_USE_AHT20) || defined(SYS_USE_ALL_EN))
	#include "AHT20.h"
#endif /* SYS_AHT20 */

#if (SYS_EN_SHT40 == 1) && (defined(SYS_USE_SHT40) || defined(SYS_USE_ALL_EN))
	#include "SHT40.h"
#endif /* SYS_SHT40 */

#if (SYS_EN_MAX7219 == 1) && (defined(SYS_USE_MAX7219) || defined(SYS_USE_ALL_EN))
#endif /* SYS_MAX7219 */

#if (SYS_EN_SSD1306 == 1) && (defined(SYS_USE_SSD1306) || defined(SYS_USE_ALL_EN))
#endif /* SYS_SSD1306 */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_INC_H */