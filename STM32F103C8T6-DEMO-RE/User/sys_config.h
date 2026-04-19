#ifndef __SYS_CONFIG_H
#define __SYS_CONFIG_H

#define SYS_USE_OS				1
#define SYS_USE_SERIAL			1
#define SYS_USE_I2C				1
#define SYS_USE_SPI				1
#define SYS_USE_RTC				1
#define SYS_USE_FLASH			1
#define SYS_USE_TH_SENSOR		1
#define SYS_USE_DISP			0

#if SYS_USE_OS
	#define SYS_USE_FREERTOS				1
	
#endif /* SYS_USE_OS */

#if SYS_USE_SERIAL
	#define SYS_USE_EASYLOGGER				1
#endif /* SYS_USE_SERIAL */

#if SYS_USE_I2C
	#define SYS_USE_I2C_BUS_STM32_STD		1
#endif /* SYS_USE_I2C */

#if SYS_USE_SPI
	#define SYS_USE_SPI_BUS_STM32_STD		1
	#define SYS_USE_SPI_BUS_STM32_STD_HW	1
#endif /* SYS_USE_SPI */

#if SYS_USE_RTC
	#define SYS_USE_DS3231			1
#endif /* SYS_USE_RTC */

#if SYS_USE_FLASH
	#define SYS_USE_W25QX			1
#endif /* SYS_USE_FLASH */

#if SYS_USE_TH_SENSOR
	#define SYS_USE_AHT20			1
	#define SYS_USE_SHT40			1
#endif /* SYS_USE_TH_SENSOR */

#if SYS_USE_DISP
	#define SYS_USE_MAX7219				1
	#define SYS_USE_SSD1306				1
#endif /* SYS_USE_DISP */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_CONFIG_H */