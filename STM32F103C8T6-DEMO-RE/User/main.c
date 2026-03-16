#include "system.h"
#include "AHT20.h"

int main(void) {
	system_init();
	AHT20_Handle_t sensor;
	sensor.i2c_addr = 0;
	AHT20_Status_t ret;
	sensor.hi2c = &swi2c_handle;
	ret = AHT20_Init(&sensor);
	log_d("sensor-init: %d \n", ret);
	log_i("sensor-temp: %f \n", sensor.temperature);
	log_i("sensor-humi: %f \n", sensor.humidity);
	
	while(1) {
		delay_ms(1000);
		ret = AHT20_Measure(&sensor);
		log_d("sensor-measure: %d \n", ret);
		log_i("sensor-temp: %f \n", sensor.temperature);
		log_i("sensor-humi: %f \n", sensor.humidity);
	}
}
