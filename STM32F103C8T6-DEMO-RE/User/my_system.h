#include "stm32f10x.h"
#include "delay.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std.h"
#include "spi_bus.h"
#include "spi_bus_stm32_std.h"
#include "AHT20.h"
#include "SHT40.h"
#include "W25QX.h"
#include "DS3231.h"

void elog_config_init(void);

extern i2c_bus_handle_t i2c_handle;
//extern i2c_bus_stm32_std_sw_config_t swi2c_cfg;
//void i2c_config_init(void);

extern spi_bus_handle_t spi_bus_handle;
//extern spi_bus_stm32_std_sw_config_t swspi_bus_cfg;
//void spi_bus_config_init(void);

extern AHT20_Handle_t aht20_handle;
//void aht20_config_init(void);

extern SHT40_Handle_t sht40_handle;
//void sht40_config_init(void);

extern DS3231_Handle_t ds3231_handle;
extern DS3231_DateTime_t ds3231_datetime;
//void ds3231_config_init(void);

//extern spi_cs_handle_t w25qx_cs_handle;
//extern spi_bus_stm32_std_cs_config_t w25qx_cs_cfg;
//extern spi_dev_handle_t w25qx_spi_handle;
extern W25QX_Handle_t w25qx_handle;
//void w25qx_config_init(void);

extern char system_time[9];
void system_sync_time(void);
void system_init(void);
void system_proc(void);
