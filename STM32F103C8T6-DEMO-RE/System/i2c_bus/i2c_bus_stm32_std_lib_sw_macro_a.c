#include "i2c_bus_stm32_std_lib_sw.h"
#include "delay.h"

#define SCL_GPIO_PORT		GPIOB
#define SCL_GPIO_PIN		GPIO_Pin_8
#define SCL_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SDA_GPIO_PORT		GPIOB
#define SDA_GPIO_PIN		GPIO_Pin_9
#define SDA_GPIO_CLK		RCC_APB2Periph_GPIOB

#define SCL_HIGH()			GPIO_SetBits(SCL_GPIO_PORT, SCL_GPIO_PIN)
#define SCL_LOW()			GPIO_ResetBits(SCL_GPIO_PORT, SCL_GPIO_PIN)
#define SCL_READ()			GPIO_ReadInputDataBit(SCL_GPIO_PORT, SCL_GPIO_PIN)

#define SDA_HIGH()			GPIO_SetBits(SDA_GPIO_PORT, SDA_GPIO_PIN)
#define SDA_LOW()			GPIO_ResetBits(SDA_GPIO_PORT, SDA_GPIO_PIN)
#define SDA_READ()			GPIO_ReadInputDataBit(SDA_GPIO_PORT, SDA_GPIO_PIN)

static inline void SWI2C_Delay(void) {
	delay_us(5);
}

static i2c_bus_status_t SWI2C_Init(void *user_data) {
	RCC_APB2PeriphClockCmd(SCL_GPIO_CLK | SDA_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	// SCL 引脚初始化
    GPIO_InitStructure.GPIO_Pin = SCL_GPIO_PIN;
    GPIO_Init(SCL_GPIO_PORT, &GPIO_InitStructure);

    // SDA 引脚初始化
    GPIO_InitStructure.GPIO_Pin = SDA_GPIO_PIN;
    GPIO_Init(SDA_GPIO_PORT, &GPIO_InitStructure);

    // 初始化状态：SCL 和 SDA 高电平（空闲状态）
    SCL_HIGH();
    SDA_HIGH();
	
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_Start(void *user_data) {
	// 检查总线是否空闲
	if(!SCL_READ() || !SDA_READ()) { return I2C_BUS_ERR_BUSY; }
	
	// SDA 从高到低，SCL 保持高
	SDA_HIGH();
	SCL_HIGH();
	SWI2C_Delay();
	
	SDA_LOW();	// SDA 下降沿
	SWI2C_Delay();
	
	SCL_LOW();	// 拉低 SCL，准备发送数据
	SWI2C_Delay();
	
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_Stop(void *user_data) {
	// SDA 从低到高，SCL 保持低 -> 高
	SCL_LOW();
	SDA_LOW();
	SWI2C_Delay();
	
	SCL_HIGH();	// 先释放 SCL
	SWI2C_Delay();
	
	SDA_HIGH();	// SDA 上升沿
	SWI2C_Delay();
	
	return I2C_BUS_OK;
}

static void SWI2C_SendACK(void *user_data) {
	SCL_LOW();
	SDA_LOW();	// SDA = 0 表示应答
	SWI2C_Delay();
	
	SCL_HIGH();	// 时钟高电平，从设备采样ACK
	SWI2C_Delay();
	
	SCL_LOW();
	SDA_HIGH();
	SWI2C_Delay();
}

static void SWI2C_SendNACK(void *user_data) {
	SCL_LOW();
	SDA_HIGH();	// SDA = 1 表示非应答
	SWI2C_Delay();
	
	SCL_HIGH();	// 时钟高电平
	SWI2C_Delay();
	
	SCL_LOW();
	SDA_HIGH();	// 保持高
	SWI2C_Delay();
}

static i2c_bus_status_t SWI2C_WaitACK(void *user_data) {
	uint8_t timeout = 0;
	
	SCL_LOW();
	SDA_HIGH();	// 释放SDA，让从机拉低
	SWI2C_Delay();

	SCL_HIGH();	// 主机释放SCL，等待从机拉低SDA
	SWI2C_Delay();

	while (SDA_READ() && timeout < 255) {
		timeout++;
		SWI2C_Delay();
    }

	SCL_LOW();	// 拉低时钟
	SWI2C_Delay();

	// 超时，无ACK
	if (timeout >= 255) { return I2C_BUS_ERR_NACK; }
	
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_WriteByte(void *user_data, uint8_t byte) {
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		SCL_LOW();	// 拉低时钟
		SWI2C_Delay();
		
		if (byte & 0x80) { SDA_HIGH(); }
		else { SDA_LOW(); }
		byte <<= 1;
		SWI2C_Delay();
		
		SCL_HIGH();
		SWI2C_Delay();
	}
	
	SCL_LOW();
	
	return SWI2C_WaitACK(user_data);
}

static i2c_bus_status_t SWI2C_ReadByte(void *user_data, uint8_t *byte, bool ack) {
	uint8_t i;
	uint8_t res = 0;
	
	SDA_HIGH();
	SWI2C_Delay();
	
	for (i = 0; i < 8; i++) {
		SCL_LOW();
		SWI2C_Delay();
		
		SCL_HIGH();  // 产生时钟上升沿
        SWI2C_Delay();
		
		res <<= 1;
		if (SDA_READ()) { res |= 0x01; }
		
		SWI2C_Delay();
	}
	
	SCL_LOW();  // 结束后拉低SCL
	
	if (ack) SWI2C_SendACK(user_data); else SWI2C_SendNACK(user_data);
	
	*byte = res;
	return I2C_BUS_OK;
}

static i2c_bus_ops_t SWI2C_OPS = {
	.init = SWI2C_Init,
    .start = SWI2C_Start,
    .stop = SWI2C_Stop,
    .write_byte = SWI2C_WriteByte,
    .read_byte = SWI2C_ReadByte
};

void i2c_bus_stm32_std_lib_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_lib_sw_config_t *cfg) {
    handle->user_data = cfg;
	handle->ops = &SWI2C_OPS;
}
