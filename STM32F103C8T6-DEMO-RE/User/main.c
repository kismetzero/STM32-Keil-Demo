#include "my_system.h"

#define LOG_TAG "system"
#include "elog.h"

int main(void) {
	system_init();
	AHT20_Measure(&aht20_handle);
	SHT40_Measure(&sht40_handle);
	while(1) {
		delay_ms(1000);
		DS3231_GetDateTime(&ds3231_handle, &ds3231_datetime);
		system_sync_time();
		AHT20_Measure(&aht20_handle);
		SHT40_Measure(&sht40_handle);
	}
}
