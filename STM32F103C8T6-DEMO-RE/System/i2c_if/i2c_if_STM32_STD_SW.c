#include "stm32f10x.h"
#include "i2c_if.h"
#include "delay.h"

#define SWI2C_SCL_GPIO_PORT		GPIOB
#define SWI2C_SCL_GPIO_PIN		GPIO_Pin_8
#define SWI2C_SCL_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SWI2C_SDA_GPIO_PORT		GPIOB
#define SWI2C_SDA_GPIO_PIN		GPIO_Pin_9
#define SWI2C_SDA_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SWI2C_SCL_H()			GPIO_SetBits(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN)
#define SWI2C_SCL_L()			GPIO_ResetBits(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN)
#define SWI2C_SCL_READ()		GPIO_ReadInputDataBit(SWI2C_SCL_GPIO_PORT, SWI2C_SCL_GPIO_PIN)

#define SWI2C_SDA_H()			GPIO_SetBits(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)
#define SWI2C_SDA_L()			GPIO_ResetBits(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)
#define SWI2C_SDA_READ()		GPIO_ReadInputDataBit(SWI2C_SDA_GPIO_PORT, SWI2C_SDA_GPIO_PIN)

static void SWI2C_Delay(void) {
	delay_us(5);
}

static i2c_if_status_t SWI2C_Init(void *user_data) {
	RCC_APB2PeriphClockCmd(SWI2C_SCL_GPIO_CLK | SWI2C_SDA_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	// SCL 引脚初始化
    GPIO_InitStructure.GPIO_Pin = SWI2C_SCL_GPIO_PIN;
    GPIO_Init(SWI2C_SCL_GPIO_PORT, &GPIO_InitStructure);

    // SDA 引脚初始化
    GPIO_InitStructure.GPIO_Pin = SWI2C_SDA_GPIO_PIN;
    GPIO_Init(SWI2C_SDA_GPIO_PORT, &GPIO_InitStructure);

    // 初始化状态：SCL 和 SDA 高电平（空闲状态）
    SWI2C_SCL_H();
    SWI2C_SDA_H();
	
	return I2C_IF_OK;
}

static i2c_if_status_t SWI2C_Start(void *user_data) {
	// 检查总线是否空闲
	if(!SWI2C_SDA_READ() || !SWI2C_SDA_READ()) { return I2C_IF_ERR_BUS_BUSY; }
	
	// SDA 从高到低，SCL 保持高
	SWI2C_SDA_H();
	SWI2C_SCL_H();
	SWI2C_Delay();
	
	SWI2C_SDA_L();	// SDA 下降沿
	SWI2C_Delay();
	
	SWI2C_SCL_L();	// 拉低 SCL，准备发送数据
	SWI2C_Delay();
	
	return I2C_IF_OK;
}

static i2c_if_status_t SWI2C_Stop(void *user_data) {
	// SDA 从低到高，SCL 保持低 -> 高
	SWI2C_SCL_L();
	SWI2C_SDA_L();
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 先释放 SCL
	SWI2C_Delay();
	
	SWI2C_SDA_H();	// SDA 上升沿
	SWI2C_Delay();
	
	return I2C_IF_OK;
}

static void SWI2C_SendACK(void) {
	SWI2C_SCL_L();
	SWI2C_SDA_L();	// SDA = 0 表示应答
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 时钟高电平，从设备采样ACK
	SWI2C_Delay();
	
	SWI2C_SCL_L();
	SWI2C_SDA_H();
	SWI2C_Delay();
}

static void SWI2C_SendNACK(void) {
	SWI2C_SCL_L();
	SWI2C_SDA_H();	// SDA = 1 表示非应答
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 时钟高电平
	SWI2C_Delay();
	
	SWI2C_SCL_L();
	SWI2C_SDA_H();	// 保持高
	SWI2C_Delay();
}

static i2c_if_status_t SWI2C_WaitACK(void) {
	uint8_t timeout = 0;
	
	SWI2C_SCL_L();
	SWI2C_SDA_H();	// 释放SDA，让从机拉低
	SWI2C_Delay();

	SWI2C_SCL_H();	// 主机释放SCL，等待从机拉低SDA
	SWI2C_Delay();

	while (SWI2C_SDA_READ() && timeout < 255) {
		timeout++;
		SWI2C_Delay();
    }

	SWI2C_SCL_L();	// 拉低时钟
	SWI2C_Delay();

	// 超时，无ACK
	if (timeout >= 255) { return I2C_IF_ERR_NACK; }
	
	return I2C_IF_OK;
}

static i2c_if_status_t SWI2C_WriteByte(void *user_data, uint8_t Byte) {
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		SWI2C_SCL_L();	// 拉低时钟
		SWI2C_Delay();
		
		if (Byte & 0x80) { SWI2C_SDA_H(); }
		else { SWI2C_SDA_L(); }
		Byte <<= 1;
		SWI2C_Delay();
		
		SWI2C_SCL_H();
		SWI2C_Delay();
	}
	
	SWI2C_SCL_L();
	
	return SWI2C_WaitACK();
}

static i2c_if_status_t SWI2C_ReadByte(void *user_data, uint8_t *Byte, bool ack) {
	uint8_t i;
	uint8_t res = 0;
	
	SWI2C_SDA_H();
	SWI2C_Delay();
	
	for (i = 0; i < 8; i++) {
		SWI2C_SCL_L();
		SWI2C_Delay();
		
		SWI2C_SCL_H();  // 产生时钟上升沿
        SWI2C_Delay();
		
		res <<= 1;
		if (SWI2C_SDA_READ()) { res |= 0x01; }
		
		SWI2C_Delay();
	}
	
	SWI2C_SCL_L();  // 结束后拉低SCL
	
	if (ack) SWI2C_SendACK(); else SWI2C_SendNACK();
	
	*Byte = res;
	return I2C_IF_OK;
}

void i2c_if_SWI2C_CreateHandle(i2c_if_handle_t *handle) {
    handle->init = SWI2C_Init;
    handle->start = SWI2C_Start;
    handle->stop = SWI2C_Stop;
    handle->write_byte = SWI2C_WriteByte;
    handle->read_byte = SWI2C_ReadByte;
}
