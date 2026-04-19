#ifndef __SYS_CORE_H
#define __SYS_CORE_H

#include "sys_config.h"

#if SYS_USE_FREERTOS
	#include "FreeRTOS.h"
	#include "semphr.h"
	#include "task.h"
#endif /* SYS_USE_FREERTOS */

#if SYS_USE_SERIAL
	#include "Serial.h"
#endif /* SYS_USE_FREERTOS */

#if SYS_USE_I2C_BUS_STM32_STD
	#include "i2c_bus_stm32_std.h"
#endif /* SYS_USE_I2C_BUS_STM32_STD */

#if SYS_USE_SPI_BUS_STM32_STD
	#include "spi_bus_stm32_std.h"
#endif /* SYS_USE_SPI_BUS_STM32_STD */

#if SYS_USE_DS3231
	#include "DS3231.h"
#endif /* SYS_USE_DS3231 */

#if SYS_USE_W25QX
	#include "W25QX.h"
#endif /* SYS_USE_W25QX */

#if SYS_USE_AHT20
	#include "AHT20.h"
#endif /* SYS_USE_AHT20 */

#if SYS_USE_SHT40
	#include "SHT40.h"
#endif /* SYS_USE_SHT40 */

#if SYS_USE_MAX7219
#endif /* SYS_USE_MAX7219 */

#if SYS_USE_SSD1306
#endif /* SYS_USE_SSD1306 */

#ifdef __cplusplus
	extern "C" {
#endif

void sys_core_init(void);

#if SYS_USE_RTC
	void sys_sync_time(void);
#endif /* SYS_USE_RTC */

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_CORE_H */