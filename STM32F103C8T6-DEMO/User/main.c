#include "stm32f10x.h"				  // Device header
#include "delay.h"
#include "USART1.h"
#include "LED.h"
#include "OLED.h"
#include "AHT20.h"
#include "EC11.h"
#include "SG90.h"

int main(void) {
	
	USART1_Init();
	LED_Init();
	LED_ON();
	USART1_printf("Hello World!\n");
	
	delay_ms(1000);
	OLED_Init();
	delay_ms(500);
	AHT20_Init();
	EC11_Init();
	SG90_Init();
	
	uint16_t angle;
	angle = EC11_GetCount();
	SG90_SetAngle(angle);
	
	OLED_ShowString(0, 0, "Cout: ", OLED_8X16);
	OLED_ShowString(0, 16, "Temp: ", OLED_8X16);
	OLED_ShowString(0, 32, "Humi: ", OLED_8X16);
	OLED_ShowNum(48, 0, EC11_GetCount(), 8, OLED_8X16);
	OLED_ShowFloatNum(48, 16, AHT20_GetTemperature(), 4, 3, OLED_8X16);
	OLED_ShowFloatNum(48, 32, AHT20_GetHumidity(), 4, 3, OLED_8X16);
	
	OLED_Update();
	delay_ms(500);
	
	while(1) {
		angle = EC11_GetCount();
		if (angle > 60000) {
			angle = 0;
			EC11_SetCount(0);
		} else if (angle > 180) {
			angle = 180;
			EC11_SetCount(180);
		}
		SG90_SetAngle(angle);
		OLED_ShowNum(48, 0, EC11_GetCount(), 8, OLED_8X16);
		OLED_ShowFloatNum(48, 16, AHT20_GetTemperature(), 4, 3, OLED_8X16);
		OLED_ShowFloatNum(48, 32, AHT20_GetHumidity(), 4, 3, OLED_8X16);
		OLED_Update();
		AHT20_Read();
		delay_ms(50);
	}
}
