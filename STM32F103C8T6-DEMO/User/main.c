#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "HX1838.h"
#include "OLED.h"

extern uint8_t HX1838_Decode_Status;
extern uint8_t HX1838_Decode_Falling;
extern uint8_t HX1838_Decode_Rising;
extern uint8_t HX1838_Decode_CNT;
extern uint8_t HX1838_Decode_POS;
extern uint32_t HX1838_Decode_Data;
extern uint16_t HX1838_Decode_LT;
extern uint16_t HX1838_Decode_HT;

int main(void) {
	Delay_Init();
	HX1838_Init();
	OLED_Init();
	OLED_ShowString(0, 0, "Stas: ", OLED_8X16);
	OLED_ShowString(0, 16, "POS: ", OLED_8X16);
	OLED_ShowString(0, 32, "CNT: ", OLED_8X16);
	OLED_ShowString(0, 48, "Data: ", OLED_8X16);
	OLED_Update();
	while(1) {
		OLED_ShowNum(48, 0, HX1838_Decode_Status, 8, OLED_8X16);
		OLED_ShowNum(48, 16, HX1838_Decode_POS, 8, OLED_8X16);
		OLED_ShowNum(48, 32, HX1838_Decode_CNT, 8, OLED_8X16);
		OLED_ShowHexNum(48, 48, HX1838_Decode_Data, 8, OLED_8X16);
		OLED_Update();
	}
}
