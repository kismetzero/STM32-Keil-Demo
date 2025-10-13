#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "HX1838.h"
#include "OLED.h"

extern uint8_t NEC_Decode_Status;
extern uint8_t NEC_Decode_POS;
extern uint32_t NEC_Decode_Data;

int main(void) {
	Delay_Init();
	HX1838_Init();
	OLED_Init();
	OLED_ShowString(0, 0, "val: ", OLED_8X16);
	OLED_ShowString(0, 16, "addr: ", OLED_8X16);
	OLED_ShowString(0, 32, "cmd: ", OLED_8X16);
	OLED_ShowString(0, 48, "cnt: ", OLED_8X16);
	OLED_Update();
	while(1) {
		OLED_ShowNum(48, 0, NEC_Data.valid, 8, OLED_8X16);
		OLED_ShowHexNum(48, 16, NEC_Data.address, 8, OLED_8X16);
		OLED_ShowHexNum(48, 32, NEC_Data.command, 8, OLED_8X16);
		OLED_ShowNum(48, 48, NEC_Data.count, 8, OLED_8X16);
		OLED_Update();
	}
}
