#include "SHT40.h"
#include "SWI2C.h"
#include "delay/delay.h"

// AHT20 I2C 地址
#define SHT40_I2C_ADDR		0x44	// 0100 0100

uint8_t SHT40_data[6];

void SHT40_Init(void) {
	SWI2C_Init();
}

uint8_t SHT40_Read(void) {
	SWI2C_Start();
	SWI2C_WriteByte(SHT40_I2C_ADDR << 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	SWI2C_WriteByte(0xFD);
	SWI2C_WaitACK();
	SWI2C_Stop();
	
	delay_ms(200);
	
	SWI2C_Start();
	SWI2C_WriteByte((SHT40_I2C_ADDR << 1) | 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	for (uint8_t i = 0; i < 6; i++) {
		SHT40_data[i] = SWI2C_ReadByte();
		if (i < 5) {
			SWI2C_SendACK();
		} else {
			SWI2C_SendNACK();
		}
	}
	SWI2C_Stop();
	return 0;
}

float SHT40_GetTemperature(void) {
	uint32_t raw_temperature;
	raw_temperature = (SHT40_data[0]  << 8) | SHT40_data[1];
	return -45.0f + 175.0f * raw_temperature / 65535.0f;
}

float SHT40_GetHumidity(void) {
	uint32_t raw_humidity;
	raw_humidity = (SHT40_data[3] << 8) | SHT40_data[4];
	return -6.0f + 125.0f * raw_humidity / 65535.0f;
}
