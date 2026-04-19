#include "sys_data.h"

#define LOG_TAG "main"
#include "elog.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static SemaphoreHandle_t mutex_lock;

#define SYS_USE_FREERTOS_TEST 1

#if SYS_USE_FREERTOS_TEST
	TaskHandle_t test_xTaskHandle;
	void test_vTask(void *pvParameters) {
		uint8_t count = 1;
		for(;;) {
			xSemaphoreTake(mutex_lock, portMAX_DELAY);
			AHT20_Measure(&aht20_handle);
			log_a("test: count=%d", count++);
			xSemaphoreGive(mutex_lock);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
	}
#endif /* SYS_USE_FREERTOS_TEST */
	
#if SYS_USE_FREERTOS
	TaskHandle_t SyncTime_xTaskHandle;
	void SyncTime_vTask(void *pvParameters) {
		uint8_t count = 1;
		for(;;) {
			xSemaphoreTake(mutex_lock, portMAX_DELAY);
			sys_sync_time();
			log_a("SyncTime: count=%d", count++);
			xSemaphoreGive(mutex_lock);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
	}
#endif /* SYS_USE_FREERTOS */

int main(void) {
	sys_core_init();
	
	mutex_lock = xSemaphoreCreateMutex();
	if (mutex_lock == NULL) {
		log_a("mutex_lock create fale");
	}
	
	#if SYS_USE_FREERTOS_TEST
		xTaskCreate(test_vTask, "testTask", 256, NULL, 1, &test_xTaskHandle);
	#endif /* SYS_USE_FREERTOS_TEST */
	
	#if SYS_USE_FREERTOS
		xTaskCreate(SyncTime_vTask, "SyncTimeTask", 128, NULL, 1, &SyncTime_xTaskHandle);
		vTaskStartScheduler();
	#endif /* SYS_USE_FREERTOS */
	
	while(1) {
	}
}
