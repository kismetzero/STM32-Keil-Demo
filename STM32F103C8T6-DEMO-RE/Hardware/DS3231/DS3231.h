#ifndef __DS3231_H
#define __DS3231_H

#include "stdint.h"

#ifdef __cplusplus
	extern "C" {
#endif

// 定义返回状态码
typedef enum {
    DS3231_STATUS_OK = 0,
	DS3231_STATUS_ERR_BUSY,
	DS3231_STATUS_ERR_I2C_ERR,
	DS3231_STATUS_ERR_INVALID_PARAM
} DS3231_Status_t;

/**
 * @brief DS3231 寄存器地址映射
 * @note 仅用于代码可读性，底层传输仍使用 uint8_t
 */
typedef enum {
    DS3231_REG_SECONDS	= 0x00,
	DS3231_REG_MINUTES	= 0x01,
	DS3231_REG_HOUR		= 0x02,
	DS3231_REG_DAY		= 0x03,
	DS3231_REG_DATE		= 0x04,
	DS3231_REG_MONTH	= 0x05,
	DS3231_REG_YEAR		= 0x06,
	
	DS3231_REG_ALARM1_SEC	= 0x07,
	DS3231_REG_ALARM1_MIN	= 0x08,
	DS3231_REG_ALARM1_HOUR	= 0x09,
	DS3231_REG_ALARM1_DYDT	= 0x0A,
	
	DS3231_REG_ALARM2_MIN	= 0x0B,
	DS3231_REG_ALARM2_HOUR	= 0x0C,
	DS3231_REG_ALARM2_DYDT	= 0x0D,
	
	DS3231_REG_CONTROL		= 0x0E,
	DS3231_REG_STATUS		= 0x0F,
	DS3231_REG_AGING		= 0x10,
	DS3231_REG_TEMP_MSB		= 0x11,
	DS3231_REG_TEMP_LSB		= 0x12,
} DS3231_RegisterAddr_t;

/**
 * @brief DS3231 控制寄存器 (0x0E) 位定义
 * @note 该寄存器控制振荡器、方波输出、温度转换及中断使能
 */
typedef enum {
    DS3231_BIT_EOSC    = 7, /* Enable Oscillator: 振荡器使能
								0: 振荡器运行 (正常模式)
								1: 振荡器停止 (省电模式)。仅在 VCC 掉电且由 VBAT 供电时有效。
								若此位为 1，所有计时和闹钟功能停止，但 SRAM 和寄存器数据保持。
							*/
    DS3231_BIT_BBSQW   = 6, /* Battery-Backed Square-Wave: 电池备份方波输出
								0: 仅当 VCC 供电时，SQW 引脚才输出方波；VBAT 供电时 SQW 为高阻态。
								1: 无论由 VCC 还是 VBAT 供电，SQW 引脚始终输出方波。
							*/
    DS3231_BIT_CONV    = 5, /* Convert Temperature: 强制温度转换
								0: 正常自动转换 (约每 64 秒一次)。
								1: 立即启动一次温度转换。转换完成后该位自动清零。
							*/
    DS3231_BIT_RS2     = 4, /* Rate Select 2: 方波频率选择位 2
								与 RS1 配合使用，设置 SQW 引脚的输出频率 (1Hz, 1kHz, 4kHz, 8kHz)。
							*/
    DS3231_BIT_RS1     = 3, /* Rate Select 1: 方波频率选择位 1
								RS2	RS1
								0	0: 1 Hz (默认，用于秒脉冲)
								0	1: 1.024 kHz
								1	0: 4.096 kHz
								1	1: 8.192 kHz
							*/
    DS3231_BIT_INTCN   = 2, /* Interrupt Control: 中断控制
								0: 当闹钟匹配时，INT/SQW 引脚输出低电平脉冲 (中断模式)。
								1: INT/SQW 引脚输出方波 (方波输出模式)，闹钟标志位仍会置起但不触发引脚中断。
								注意：若 A1IE/A2IE 未使能，即使 INTCN=0 也不会触发中断。
							*/
    DS3231_BIT_A2IE    = 1, /* Alarm 2 Interrupt Enable: 闹钟 2 中断使能
								0: 禁止闹钟 2 触发中断。
								1: 允许闹钟 2 匹配时置位 A2F 并触发 INT 引脚 (需 INTCN=0)。
							*/
    DS3231_BIT_A1IE    = 0  /* Alarm 1 Interrupt Enable: 闹钟 1 中断使能
								0: 禁止闹钟 1 触发中断。
								1: 允许闹钟 1 匹配时置位 A1F 并触发 INT 引脚 (需 INTCN=0)。
							*/
} DS3231_ControlBits_t;

// 状态寄存器 (0x0F) 位定义
typedef enum {
    DS3231_BIT_OSF     = 7, /* Oscillator Stop Flag: 停振标志
								[触发条件]: 当芯片首次上电、电压过低导致停振、或从电池模式切换回主电源时，
											若检测到振荡器曾停止工作，此位会被硬件置 '1'。
								[影响]: 此时时间数据可能无效，需重新校准时间。
								[操作]: 软件读取到此位为 '1' 后，应重新设置时间寄存器，并写入 '0' 清除此标志。
										若不清除，下次掉电重启后该位可能依然保持为 '1'。
							*/
	// Bit 6: Reserved
	// Bit 5: Reserved
	// Bit 4: Reserved
    DS3231_BIT_EN32KHZ = 3, /* Enable 32kHz Output: 32kHz 输出使能
								[功能]: 控制是否从 32kHz 引脚输出标准的 32.768 kHz 正弦波/方波。
										1: 启用输出 (默认状态，只要电池有电就会输出)。
										0: 禁用输出。
								[应用场景]: 若系统不需要外部 32kHz 时钟源，可写 '0' 关闭以节省微安级电流。
							*/
	// Bit 2: Reserved
    DS3231_BIT_A2F     = 1, /* Alarm 2 Flag: 闹钟 2 中断标志
								[触发条件]: 当当前时间与闹钟 2 的设置时间匹配时，硬件自动置 '1'。
								[行为]:
										1 - 若控制寄存器的 INTCN=0 (中断模式) 且 A2IE=1，此位置 '1' 会拉低 INT/SQW 引脚。
										2 - 若控制寄存器的 INTCN=1 (方波模式)，此位仍会置 '1'，但不会影响引脚电平。
								[操作]: 必须软件写入 '0' 清除。若不清除，中断引脚将一直保持低电平 (在中断模式下)。
							*/
    DS3231_BIT_A1F     = 0  /* Alarm 1 Flag: 闹钟 1 中断标志
								[触发条件]: 当当前时间与闹钟 1 的设置时间匹配时，硬件自动置 '1'。
								[行为]:
										1 - 若控制寄存器的 INTCN=0 (中断模式) 且 A1IE=1，此位置 '1' 会拉低 INT/SQW 引脚。
										2 - 若控制寄存器的 INTCN=1 (方波模式)，此位仍会置 '1'，但不会影响引脚电平。
								[操作]: 必须软件写入 '0' 清除。若不清除，中断引脚将一直保持低电平 (在中断模式下)。
								[注意]: 如果同时触发了闹钟 1 和 2，两个标志位都会置 '1'，需分别清除。
							*/
} DS3231_StatusBits_t;

// Bit 6: DY/DT 标志位
// 0 = 匹配日期 (Date, 1-31)
// 1 = 匹配星期 (Day, 1-7)
#define DS3231_ALM_DYDT_BIT     (1 << 6)

// Bit 7: A1M4 / A2M3 掩码位 (Alarm Mask)
// 0 = 必须匹配该寄存器的值才能触发闹钟
// 1 = 忽略该寄存器 (即“通配符”，无论该位是多少都触发)
#define DS3231_ALM_MASK_BIT     (1 << 7)

typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
} DS3231_Time_t;

// DS3231 默认 I2C 地址
#define DS3231_DEFAULT_I2C_ADDR	0x68 // 0110 1000

// 定义 DS3231 设备句柄结构体
typedef struct {
	void *hi2c;
	uint8_t i2c_addr;
} DS3231_Handle_t;

DS3231_Status_t DS3231_Init(DS3231_Handle_t *dev, void *hi2c, uint8_t i2c_addr);
DS3231_Status_t DS3231_Reset(DS3231_Handle_t *dev);
DS3231_Status_t DS3231_ReadStatus(DS3231_Handle_t *dev);
DS3231_Status_t DS3231_ReadControl(DS3231_Handle_t *dev);
DS3231_Status_t DS3231_ReadTime(DS3231_Handle_t *dev);

/**
 * @brief 将 BCD 码转换为十进制整数
 * @param val BCD 格式的值
 * @return 十进制整数值
 */
static inline uint8_t DS3231_BCD2DEC(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

/**
 * @brief 将十进制整数转换为 BCD 码
 * @param val 十进制整数值 (0-99)
 * @return BCD 格式的值
 */
static inline uint8_t DS3231_DEC2BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

#ifdef __cplusplus
}
#endif

#endif  /* __DS3231_H */