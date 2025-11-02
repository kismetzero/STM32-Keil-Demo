/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

void PWM_OCDeInit(void);
void PWM_OCInit(void);
void PWM_OCSetCompare(uint16_t comp);
void PWM_OCSetPrescaler(uint16_t presc);
void PWM_OCSetFreq(uint16_t freq);
void PWM_OCSetDuty(uint16_t duty);

void PWM_ICInit(void);
uint32_t PWM_ICGetCapture(void);
uint32_t PWM_ICGetCapture2(void);
uint32_t PWM_ICGetFreq(void);
uint32_t PWM_ICGetDuty(void);

//PWM TEST CODE
/*
	Delay_Init();
	PWM_OCInit();
	PWM_ICInit();
	OLED_Init();
	OLED_ShowString(0, 0, "Freq: ", OLED_8X16);
	OLED_ShowString(0, 16, "Duty: ", OLED_8X16);
	OLED_ShowString(0, 32, "Cap1: ", OLED_8X16);
	OLED_ShowString(0, 48, "Cap2: ", OLED_8X16);
	OLED_Update();
	while(1) {
		OLED_ShowNum(48, 0, PWM_ICGetFreq(), 6, OLED_8X16);
		OLED_ShowNum(48, 16, PWM_ICGetDuty(), 6, OLED_8X16);
		OLED_ShowNum(48, 32, PWM_ICGetCapture(), 6, OLED_8X16);
		OLED_ShowNum(48, 48, PWM_ICGetCapture2(), 6, OLED_8X16);
		OLED_Update();
		Delay_us(1000);
	}
*/

#ifdef __cplusplus
}
#endif

#endif  /* __PWM_H */