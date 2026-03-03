#include "stm32f10x.h"
#include "AHT20.h"
#include "SWI2C.h"
#include "delay.h"

// AHT20 I2C 地址
#define AHT20_I2C_ADDR		0x38	// 0011 1000

uint8_t AHT20_data[6];

uint8_t AHT20_Init(void) {
	SWI2C_Init();
	delay_ms(50);
	
	SWI2C_Start();
	SWI2C_WriteByte(AHT20_I2C_ADDR << 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	SWI2C_WriteByte(0xBE);
	SWI2C_WaitACK();
	SWI2C_WriteByte(0x08);
	SWI2C_WaitACK();
	SWI2C_WriteByte(0x00);
	SWI2C_WaitACK();
	SWI2C_Stop();
	
	delay_ms(50);
	
	SWI2C_Start();
	SWI2C_WriteByte((AHT20_I2C_ADDR << 1) | 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	for (uint8_t i = 0; i < 6; i++) {
		AHT20_data[i] = SWI2C_ReadByte();
		SWI2C_SendACK();
		if (i == 6 - 1) {
			SWI2C_SendNACK();
		}
	}
	SWI2C_Stop();
	
	// 检查校准位
	if (!(AHT20_data[0] & (1 << 3))) {
		return -1;
	}
	
	return 0;
}

uint8_t AHT20_Reset(void) {
	uint8_t status = 0;
	SWI2C_Start();
	SWI2C_WriteByte(AHT20_I2C_ADDR << 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	SWI2C_WriteByte(0xBA);
	status = SWI2C_WaitACK();
	SWI2C_Stop();
	return status ? -1 : 0;
}

uint8_t AHT20_Read(void) {
	SWI2C_Start();
	SWI2C_WriteByte(AHT20_I2C_ADDR << 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	SWI2C_WriteByte(0xAC);
	SWI2C_WaitACK();
	SWI2C_WriteByte(0x33);
	SWI2C_WaitACK();
	SWI2C_WriteByte(0x00);
	SWI2C_WaitACK();
	SWI2C_Stop();
	
	delay_ms(80);
	
	SWI2C_Start();
	SWI2C_WriteByte((AHT20_I2C_ADDR << 1) | 1);
	if (SWI2C_WaitACK()) {
		SWI2C_Stop();
		return -1;
	}
	for (uint8_t i = 0; i < 6; i++) {
		AHT20_data[i] = SWI2C_ReadByte();
		if (i < 5) {
			SWI2C_SendACK();
		} else {
			SWI2C_SendNACK();
		}
	}
	SWI2C_Stop();
	
	// 检查是否忙
	if (AHT20_data[0] & (1 << 7)) {
		return -1;
	}
	
	return 0;
}

float AHT20_GetTemperature(void) {
	uint32_t raw_temperature;
	raw_temperature = (((AHT20_data[3] & 0x0F) << 16) | (AHT20_data[4] << 8)) | AHT20_data[5];
	return (((raw_temperature * 200.0f) / (1 << 20)) - 50.0f);
}

float AHT20_GetHumidity(void) {
	uint32_t raw_humidity;
	raw_humidity = ((AHT20_data[1] << 12) | (AHT20_data[2] << 4)) | (AHT20_data[3] >> 4);
	return ((raw_humidity * 100.0f) / (1 << 20));
}
