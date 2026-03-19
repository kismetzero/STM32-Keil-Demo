#include "system.h"
#include "AHT20.h"

int main(void) {
	system_init();
	w25qx_test();
	
	while(1) {
		delay_ms(1000);
		w25qx_test();
	}
}
