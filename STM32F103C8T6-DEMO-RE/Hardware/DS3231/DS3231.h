#ifndef __DS3231_H
#define __DS3231_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    DS3231_STATUS_OK = 0,
	DS3231_STATUS_ERR,
	DS3231_STATUS_ERR_INVALID_PARAM,
	DS3231_STATUS_ERR_NO_INIT,
	DS3231_STATUS_ERR_I2C_ERR,
	DS3231_STATUS_ERR_BUSY,
} DS3231_Status_t;

/*
 * @brief DS3231 寄存器地址
 */
#define DS3231_REG_SEC			0x00U	// 秒钟 (00-59)
#define DS3231_REG_MIN			0x01U	// 分钟 (00-59)
#define DS3231_REG_HOUR			0x02U	// 时钟 (00-23 or 01-12 + AM/PM)
#define DS3231_REG_DAY			0x03U	// 星期 (01-07) week day 
#define DS3231_REG_DATE			0x04U	// 日期 (01-31) month day 
#define DS3231_REG_MONTH		0x05U	// 月份 (01-12 + Century 世纪位)
#define DS3231_REG_YEAR			0x06U	// 年份 (00-99)

#define DS3231_REG_ALM1_SEC		0x07U	// 秒钟 (00-59)
#define DS3231_REG_ALM1_MIN		0x08U	// 分钟 (00-59)
#define DS3231_REG_ALM1_HOUR	0x09U	// 时钟 (00-23 or 01-12 + AM/PM)
#define DS3231_REG_ALM1_DYDT	0x0AU	// 星期 (Day, 01-07) 日期 (Date, 01-31)

#define DS3231_REG_ALM2_MIN		0x0BU	// 分钟 (00-59)
#define DS3231_REG_ALM2_HOUR	0x0CU	// 时钟 (00-23 or 01-12 + AM/PM)
#define DS3231_REG_ALM2_DYDT	0x0DU	// 星期 (Day, 01-07) 日期 (Date, 01-31)

#define DS3231_REG_CONTROL		0x0EU	// 控制	Control
#define DS3231_REG_STATUS		0x0FU	// 控制 + 状态 Control + Status
#define DS3231_REG_AGING		0x10U	// 老化偏移 Aging Offest
#define DS3231_REG_TEMP_MSB		0x11U	// 温度高位
#define DS3231_REG_TEMP_LSB		0x12U	// 温度低位

#define DS3231_MASK_SEC			(0x7FU)		// 秒钟数据掩码, 低 7 位是数据 (00-59)
#define DS3231_MASK_MIN			(0x7FU)		// 分钟数据掩码, 低 7 位是数据 (00-59)
#define DS3231_MASK_HOUR_24H	(0x3FU)     // 24h 模式掩码，低 6 位是数据 (00-23)
#define DS3231_MASK_HOUR_12H	(0x1FU)     // 12h 模式掩码，低 5 位是数据 (01-12)
#define DS3231_MASK_DAY			(0x07U)		// 日期数据掩码, 低 6 位是数据 (01-07)
#define DS3231_MASK_DATE		(0x3FU)		// 日期数据掩码, 低 6 位是数据 (01-31)
#define DS3231_MASK_MONTH		(0x1FU)		// 月份数据掩码, 低 5 位是数据 (01-12)
#define DS3231_MASK_YEAR		(0xFFU)		// 年份数据掩码, 低 8 位是数据 (00-99)

#define DS3231_MASK_ALM_SEC		(0x7FU)		// 秒钟数据掩码, 低 7 位是数据 (00-59)
#define DS3231_MASK_ALM_MIN		(0x7FU)		// 分钟数据掩码, 低 7 位是数据 (00-59)
#define DS3231_MASK_ALM_24H			(0x3FU)     // 24h 模式掩码，低 6 位是数据 (00-23)
#define DS3231_MASK_ALM_12H			(0x1FU)     // 12h 模式掩码，低 5 位是数据 (01-12)
#define DS3231_MASK_ALM_DYDT		(0x3FU)		// 星期日期数据掩码, 低 6 位是数据

#define DS3231_BIT_HOUR_EN12H		(1U << 6)	// 0: 24h	1: 12h
#define DS3231_BIT_HOUR_PM			(1U << 5)	// 0: AM	1: PM	(仅在 12h 模式下有效)

#define DS3231_BIT_MONTH_CENTURY	(1U << 7)	// 0: 20xx	1: 21xx

#define DS3231_BIT_ALM_IGNORE		(1U << 7)	// 0: 必须匹配该寄存器的值才能触发闹钟		1: 忽略该寄存器
#define DS3231_BIT_ALM_EN12H		(1U << 6)	// 0: 24h	1: 12h
#define DS3231_BIT_ALM_AM_PM		(1U << 5)	// 0: AM	1: PM	(仅在 12h 模式下有效)
#define DS3231_BIT_ALM_DYDT			(1U << 6)	// 0 = 匹配日期 (Date, 01-31)	1 = 匹配星期 (Day, 01-07)

/* Enable Oscillator: 振荡器使能
 * 0: 振荡器运行 (正常模式)
 * 1: 振荡器停止 (省电模式)。仅在 VCC 掉电且由 VBAT 供电时有效。
 * 若此位为 1，所有计时和闹钟功能停止，但 SRAM 和寄存器数据保持。
 */
#define DS3231_BIT_CTRL_EOSC	(1U << 7)

/* Battery-Backed Square-Wave: 电池备份方波输出
 * 0: 仅当 VCC 供电时，SQW 引脚才输出方波；VBAT 供电时 SQW 为高阻态。
 * 1: 无论由 VCC 还是 VBAT 供电，SQW 引脚始终输出方波。
 */
#define DS3231_BIT_CTRL_BBSQW	(1U << 6)

/* Convert Temperature: 强制温度转换
 * 0: 正常自动转换 (约每 64 秒一次)。
 * 1: 立即启动一次温度转换。转换完成后该位自动清零。
 */
#define DS3231_BIT_CTRL_CONV	(1U << 5)

/* Rate Select 2: 方波频率选择位 2
 * 0: 与 RS1 配合使用，设置 SQW 引脚的输出频率 (1Hz, 1kHz, 4kHz, 8kHz)。
 */
#define DS3231_BIT_CTRL_RS2		(1U << 4)
/* Rate Select 1: 方波频率选择位 1
 * RS2	RS1
 * 0	0: 1 Hz (默认，用于秒脉冲)
 * 0	1: 1.024 kHz
 * 1	0: 4.096 kHz
 * 1	1: 8.192 kHz
 */
#define DS3231_BIT_CTRL_RS1		(1U << 3)

/* Interrupt Control: 中断控制
 * 0: INT/SQW 引脚输出方波 (方波输出模式)，闹钟标志位仍会置起但不触发引脚中断。
 * 1: 当闹钟匹配时，INT/SQW 引脚输出低电平脉冲 (中断模式)。
 * 注意：若 A1IE/A2IE 未使能，即使 INTCN=1 也不会触发中断。
 */
#define DS3231_BIT_CTRL_INTCN	(1U << 2)

/* Alarm 2 Interrupt Enable: 闹钟 2 中断使能
 * 0: 禁止闹钟 2 触发中断。
 * 1: 允许闹钟 2 匹配时置位 A2F 并触发 INT 引脚 (需 INTCN=1)。
 */
#define DS3231_BIT_CTRL_A2IE	(1U << 1)

/* Alarm 1 Interrupt Enable: 闹钟 1 中断使能
 * 0: 禁止闹钟 1 触发中断。
 * 1: 允许闹钟 1 匹配时置位 A1F 并触发 INT 引脚 (需 INTCN=1)。
 */
#define DS3231_BIT_CTRL_A1IE	(1U << 0)  


/* Oscillator Stop Flag: 停振标志
 * [触发条件]: 当芯片首次上电、电压过低导致停振、或从电池模式切换回主电源时，
 * 				若检测到振荡器曾停止工作，此位会被硬件置 '1'。
 * [影响]: 此时时间数据可能无效，需重新校准时间。
 * [操作]: 软件读取到此位为 '1' 后，应重新设置时间寄存器，并写入 '0' 清除此标志。
 * 			若不清除，下次掉电重启后该位可能依然保持为 '1'。
 */
#define DS3231_BIT_STATUS_OSF		(1U << 7)

/* Enable 32kHz Output: 32kHz 输出使能
 * [功能]: 控制是否从 32kHz 引脚输出标准的 32.768 kHz 正弦波/方波。
 * 1: 启用输出 (默认状态，只要电池有电就会输出)。
 * 0: 禁用输出。
 * [应用场景]: 若系统不需要外部 32kHz 时钟源，可写 '0' 关闭以节省微安级电流。
 */
#define DS3231_BIT_STATUS_EN32KHZ	(1U << 3)

/* TCXO Busy: TCXO 功能忙标志
 * 
 * 
 */
#define DS3231_BIT_STATUS_BSY		(1U << 2)

/* Alarm 2 Flag: 闹钟 2 中断标志
 * [触发条件]: 当当前时间与闹钟 2 的设置时间匹配时，硬件自动置 '1'。
 * [行为]:
 * 		0 - 若控制寄存器的 INTCN=0 (中断模式) 且 A2IE=1，此位置 '1' 会拉低 INT/SQW 引脚。
 * 		1 - 若控制寄存器的 INTCN=1 (方波模式)，此位仍会置 '1'，但不会影响引脚电平。
 * [操作]: 必须软件写入 '0' 清除。若不清除，中断引脚将一直保持低电平 (在中断模式下)。
 */
#define DS3231_BIT_STATUS_A2F		(1U << 1)

/* Alarm 1 Flag: 闹钟 1 中断标志
 * [触发条件]: 当当前时间与闹钟 1 的设置时间匹配时，硬件自动置 '1'。
 * [行为]:
 * 		0 - 若控制寄存器的 INTCN=0 (中断模式) 且 A1IE=1，此位置 '1' 会拉低 INT/SQW 引脚。
 * 		1 - 若控制寄存器的 INTCN=1 (方波模式)，此位仍会置 '1'，但不会影响引脚电平。
 * [操作]: 必须软件写入 '0' 清除。若不清除，中断引脚将一直保持低电平 (在中断模式下)。
 * [注意]: 如果同时触发了闹钟 1 和 2，两个标志位都会置 '1'，需分别清除。
 */
#define DS3231_BIT_STATUS_A1F		(1U << 0)  

typedef enum {
    DS3231_SQW_1HZ		= 0,	// RS2=0, RS1=0
	DS3231_SQW_1024HZ	= 1,	// RS2=0, RS1=1
	DS3231_SQW_4096HZ	= 2,	// RS2=1, RS1=0
	DS3231_SQW_8192HZ	= 3		// RS2=1, RS1=1
} DS3231_SQW_Mode_t;

typedef struct DS3231_DateTime_s DS3231_DateTime_t;

struct DS3231_DateTime_s {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t week;
	uint8_t date;
	uint8_t month;
	uint16_t year;
	bool en12h;
	bool pm;
//	bool century;
};

// DS3231 默认 I2C 地址
#define DS3231_DEFAULT_I2C_ADDR	0x68 // 0110 1000

// 定义 DS3231 设备句柄结构体
typedef struct {
	void		*hi2c;
	uint8_t		i2c_addr;
	uint8_t		inited;
} DS3231_Handle_t;

DS3231_Status_t DS3231_Init(DS3231_Handle_t *handle, void *hi2c, uint8_t i2c_addr);
DS3231_Status_t DS3231_SoftwareReset(DS3231_Handle_t *handle);
DS3231_Status_t DS3231_ClearOSF(DS3231_Handle_t *handle);

DS3231_Status_t DS3231_ReadControlRegister(DS3231_Handle_t *handle, uint8_t *data);
DS3231_Status_t DS3231_ReadStatusRegister(DS3231_Handle_t *handle, uint8_t *data);

DS3231_Status_t DS3231_GetDateTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);
DS3231_Status_t DS3231_SetDateTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);

DS3231_Status_t DS3231_GetTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);
DS3231_Status_t DS3231_SetTime(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);

DS3231_Status_t DS3231_GetDate(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);
DS3231_Status_t DS3231_SetDate(DS3231_Handle_t *handle, DS3231_DateTime_t *dt);

/**
 * @brief 将 BCD 码转换为十进制整数
 * @param val BCD 格式的值
 * @return 十进制整数值
 */
//static inline uint8_t DS3231_BCD2DEC(uint8_t val) {
//    return ((val >> 4) * 10) + (val & 0x0F);
//}

/**
 * @brief 将十进制整数转换为 BCD 码
 * @param val 十进制整数值 (0-99)
 * @return BCD 格式的值
 */
//static inline uint8_t DS3231_DEC2BCD(uint8_t val) {
//    return ((val / 10) << 4) | (val % 10);
//}

#ifdef __cplusplus
}
#endif

#endif  /* __DS3231_H */