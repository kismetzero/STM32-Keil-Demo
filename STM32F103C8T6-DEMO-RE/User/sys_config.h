#ifndef __SYS_CONFIG_H
#define __SYS_CONFIG_H

#define SYS_EN_OS				1
#define SYS_EN_SERIAL			1
#define SYS_EN_I2C				1
#define SYS_EN_SPI				1
#define SYS_EN_RTC				1
#define SYS_EN_FLASH			1
#define SYS_EN_TH_SENSOR		1
#define SYS_EN_DISP			0

#if SYS_EN_OS
	#define SYS_EN_FREERTOS				1
#endif /* SYS_EN_OS */

#if SYS_EN_SERIAL
	#define SYS_EN_EASYLOGGER				1
#endif /* SYS_EN_SERIAL */

#if SYS_EN_I2C
	#define SYS_EN_I2C_BUS_STM32_STD		1
#endif /* SYS_EN_I2C */

#if SYS_EN_SPI
	#define SYS_EN_SPI_BUS_STM32_STD		1
	#define SYS_EN_SPI_BUS_STM32_STD_HW	1
#endif /* SYS_EN_SPI */

#if SYS_EN_RTC
	#define SYS_EN_DS3231			1
#endif /* SYS_EN_RTC */

#if SYS_EN_FLASH
	#define SYS_EN_W25QX			1
#endif /* SYS_EN_FLASH */

#if SYS_EN_TH_SENSOR
	#define SYS_EN_AHT20			1
	#define SYS_EN_SHT40			1
#endif /* SYS_EN_TH_SENSOR */

#if SYS_EN_DISP
	#define SYS_EN_MAX7219				1
	#define SYS_EN_SSD1306				1
#endif /* SYS_EN_DISP */

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_CONFIG_H */