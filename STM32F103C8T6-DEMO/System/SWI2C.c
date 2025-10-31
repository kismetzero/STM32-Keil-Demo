#include "SWI2C.h"

void SWI2C_Delay(void) {

}

void SWI2C_Init(void) {
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
}

void SWI2C_Start(void) {
	// SDA 从高到低，SCL 保持高
	SWI2C_SDA_H();
	SWI2C_SCL_H();
	SWI2C_Delay();
	
	SWI2C_SDA_L();	// SDA 下降沿
	SWI2C_Delay();
	
	SWI2C_SCL_L();	// 拉低 SCL，准备发送数据
	SWI2C_Delay();
}

void SWI2C_Stop(void) {
	// SDA 从低到高，SCL 保持低 -> 高
	SWI2C_SDA_L();
	SWI2C_SCL_H();
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 先释放 SCL
	SWI2C_Delay();
	
	SWI2C_SDA_H();	// SDA 上升沿
	SWI2C_Delay();
}

void SWI2C_SendACK(void) {
	SWI2C_SCL_L();
	SWI2C_SDA_L();	// SDA = 0 表示应答
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 时钟高电平，从设备采样ACK
	SWI2C_Delay();
	
	SWI2C_SCL_L();
	SWI2C_SDA_H();
	SWI2C_Delay();
}

void SWI2C_SendNoACK(void) {
	SWI2C_SCL_L();
	SWI2C_SDA_H();	// SDA = 1 表示非应答
	SWI2C_Delay();
	
	SWI2C_SCL_H();	// 时钟高电平
	SWI2C_Delay();
	
	SWI2C_SCL_L();
	SWI2C_SDA_H();	// 保持高
	SWI2C_Delay();
}

uint8_t SWI2C_WaitACK(void) {
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

	if (timeout >= 255) {
		return 1;	// 超时，无ACK
    }
	return 0;		// 收到ACK
}

void SWI2C_WriteByte(uint8_t Byte) {
	uint8_t i;
	
	for(i = 0; i < 8; i++) {
		SWI2C_SCL_L();	// 拉低时钟
		SWI2C_Delay();
		
		if(Byte & 0x80) {
			SWI2C_SDA_H();
		} else {
			SWI2C_SDA_L();
		}
		Byte <<= 1;
		SWI2C_Delay();
		
		SWI2C_SCL_H();
		SWI2C_Delay();
		
		SWI2C_SCL_L();
	}
	SWI2C_SCL_L();
}

uint8_t SWI2C_ReadByte(void) {
	uint8_t i;
	uint8_t Byte = 0;
	
	SWI2C_SDA_H();
	SWI2C_Delay();
	
	for(i = 0; i < 8; i++) {
		SWI2C_SCL_L();
		SWI2C_Delay();
		
		SWI2C_SCL_H();  // 产生时钟上升沿
        SWI2C_Delay();
		
		Byte <<= 1;
		if (SWI2C_SDA_READ()) {
            Byte |= 0x01;
        }
		
		SWI2C_Delay();
	}
	SWI2C_SCL_L();  // 结束后拉低SCL
	return Byte;
}
