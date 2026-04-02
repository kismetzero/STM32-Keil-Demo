#include "stm32f10x.h"
#include "delay.h"
#include "i2c_bus.h"
#include "i2c_bus_stm32_std.h"
#include "spi_bus.h"
#include "spi_bus_stm32_std.h"
#include "DS3231.h"
#include "AHT20.h"
#include "SHT40.h"
#include "W25QX.h"

extern i2c_bus_handle_t i2c_handle;
extern spi_bus_handle_t spi_bus_handle;

extern DS3231_Handle_t ds3231_handle;
extern DS3231_DateTime_t ds3231_datetime;

extern AHT20_Handle_t aht20_handle;
extern SHT40_Handle_t sht40_handle;

extern W25QX_Handle_t w25qx_handle;

extern char system_time[9];
void system_sync_time(void);

void system_init(void);
void system_proc(void);
