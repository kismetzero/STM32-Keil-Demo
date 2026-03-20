#include "system.h"

int main(void) {
	system_init();
	w25qx_test();
	AHT20_Measure(&aht20_handle);
	
	while(1) {
		delay_ms(1000);
		w25qx_test();
		AHT20_Measure(&aht20_handle);
	}
}
