#include "my_system.h"

#define LOG_TAG "system"
#include "elog.h"

int main(void) {
	system_init();
	
	while(1) {
		delay_ms(1000);
		system_sync_time();
		AHT20_Measure(&aht20_handle);
	}
}
