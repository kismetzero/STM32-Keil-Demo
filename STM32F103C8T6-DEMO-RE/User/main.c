#include "system.h"

int main(void) {
	system_init();
	AHT20_Measure(&aht20_handle);
	SHT40_Measure(&sht40_handle);
	DS3231_ReadStatus(&ds3231_handle);
	DS3231_ReadControl(&ds3231_handle);
	DS3231_ReadTime(&ds3231_handle);
	
	while(1) {
		delay_ms(1000);
		AHT20_Measure(&aht20_handle);
		SHT40_Measure(&sht40_handle);
		DS3231_ReadStatus(&ds3231_handle);
		DS3231_ReadControl(&ds3231_handle);
		DS3231_ReadTime(&ds3231_handle);
	}
}
