#include "SHT40.h"
#include "SWI2C.h"
#include "delay.h"

// AHT20 I2C 地址
#define SHT40_I2C_ADDR			0x44

uint8_t SHT40_data[6];

void SHT40_Init(void) {
	SWI2C_Init();
}

void SHT40_Read(void) {
	SWI2C_Start();
	SWI2C_WriteByte(SHT40_I2C_ADDR << 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return;
	}
	SWI2C_WriteByte(0xFD);
	SWI2C_WaitACK();
	SWI2C_Stop();
	
	delay_ms(200);
	
	SWI2C_Start();
	SWI2C_WriteByte((SHT40_I2C_ADDR << 1) | 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return;
	}
	for (uint8_t i = 0; i < 6; i++) {
		SHT40_data[i] = SWI2C_ReadByte();
		SWI2C_SendACK();
		if (i == 6 - 1) {
			SWI2C_SendNACK();
		}
	}
	SWI2C_Stop();
}
