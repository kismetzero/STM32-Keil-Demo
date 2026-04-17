#include "my_system.h"

#include "FreeRTOS.h"
#include "task.h"

#define LOG_TAG "main"
#include "elog.h"

// 定义任务句柄
TaskHandle_t xTaskHandle1 = NULL;

// 任务1函数
void vTask1(void *pvParameters)
{
    while(1)
    {
		system_sync_time();
        log_a("task1: %s", system_time);
		W25QX_ReadID(&w25qx_handle);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 延时500ms
    }
}

int main(void) {
	int i = 0;
	system_init();
	xTaskCreate(vTask1, "Task1", 128, NULL, 1, &xTaskHandle1);
	vTaskStartScheduler();
	while(1) {
		
	}
}
