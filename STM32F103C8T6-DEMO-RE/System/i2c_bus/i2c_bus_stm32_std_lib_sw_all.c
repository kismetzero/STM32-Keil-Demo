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
#define SCL_WRITE(X)		GPIO_WriteBit(SCL_GPIO_PORT, SCL_GPIO_PIN, X ? Bit_SET : Bit_RESET)

#define SDA_HIGH()			GPIO_SetBits(SDA_GPIO_PORT, SDA_GPIO_PIN)
#define SDA_LOW()			GPIO_ResetBits(SDA_GPIO_PORT, SDA_GPIO_PIN)
#define SDA_READ()			GPIO_ReadInputDataBit(SDA_GPIO_PORT, SDA_GPIO_PIN)
#define SDA_WRITE(X)		GPIO_WriteBit(SDA_GPIO_PORT, SDA_GPIO_PIN, X ? Bit_SET : Bit_RESET)

static i2c_bus_status_t SWI2C_Init(void *user_data) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;

	RCC_APB2PeriphClockCmd(cfg->scl_gpio_clk | cfg->sda_gpio_clk, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// SCL 引脚初始化
    GPIO_InitStructure.GPIO_Pin = cfg->scl_gpio_pin;
    GPIO_Init(cfg->scl_gpio_port, &GPIO_InitStructure);
    // SDA 引脚初始化
    GPIO_InitStructure.GPIO_Pin = cfg->sda_gpio_pin;
    GPIO_Init(cfg->sda_gpio_port, &GPIO_InitStructure);
    // 初始化状态：SCL 和 SDA 高电平（空闲状态）
    scl_write(cfg, 1);
    sda_write(cfg, 1);

	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_Start(void *user_data) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	// 检查总线是否空闲
	if(!scl_read(cfg) || !sda_read(cfg)) { return I2C_BUS_ERR_BUSY; }

	// SDA 从高到低，SCL 保持高
	sda_write(cfg, 1);
	scl_write(cfg, 1);
	SWI2C_Delay();

	sda_write(cfg, 0);	// SDA 下降沿
	SWI2C_Delay();

	scl_write(cfg, 0);	// 拉低 SCL，准备发送数据
	SWI2C_Delay();

	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_Stop(void *user_data) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	// SDA 从低到高，SCL 保持低 -> 高
	scl_write(cfg, 0);
	sda_write(cfg, 0);
	SWI2C_Delay();

	scl_write(cfg, 1);	// 先释放 SCL
	SWI2C_Delay();

	sda_write(cfg, 1);	// SDA 上升沿
	SWI2C_Delay();

	return I2C_BUS_OK;
}

static void SWI2C_SendACK(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	scl_write(cfg, 0);
	sda_write(cfg, 0);	// SDA = 0 表示应答
	SWI2C_Delay();

	scl_write(cfg, 1);	// 时钟高电平，从设备采样ACK
	SWI2C_Delay();

	scl_write(cfg, 0);
	sda_write(cfg, 1);
	SWI2C_Delay();
}

static void SWI2C_SendNACK(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	scl_write(cfg, 0);
	sda_write(cfg, 1);	// SDA = 1 表示非应答
	SWI2C_Delay();

	scl_write(cfg, 1);	// 时钟高电平
	SWI2C_Delay();

	scl_write(cfg, 0);
	sda_write(cfg, 1);	// 保持高
	SWI2C_Delay();
}

static i2c_bus_status_t SWI2C_WaitACK(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	uint8_t timeout = 0;

	scl_write(cfg, 0);
	sda_write(cfg, 1);	// 释放SDA，让从机拉低
	SWI2C_Delay();

	scl_write(cfg, 1);	// 主机释放SCL，等待从机拉低SDA
	SWI2C_Delay();

	while (sda_read(cfg) && timeout < 255) {
		timeout++;
		SWI2C_Delay();
    }

	scl_write(cfg, 0);	// 拉低时钟
	SWI2C_Delay();
	// 超时，无ACK
	if (timeout >= 255) { return I2C_BUS_ERR_NACK; }
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_WriteByte(void *user_data, uint8_t byte) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	for (uint8_t i = 0; i < 8; i++) {
		scl_write(cfg, 0);	// 拉低时钟
		SWI2C_Delay();

		if (byte & 0x80) { sda_write(cfg, 1); }
		else { sda_write(cfg, 0); }
		byte <<= 1;
		SWI2C_Delay();

		scl_write(cfg, 1);
		SWI2C_Delay();
	}

	scl_write(cfg, 0);
	return SWI2C_WaitACK(cfg);
}

static i2c_bus_status_t SWI2C_ReadByte(void *user_data, uint8_t *byte, bool ack) {
	if (user_data == NULL || byte == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	uint8_t res = 0x00;

	sda_write(cfg, 1);
	SWI2C_Delay();

	for (uint8_t i = 0; i < 8; i++) {
		scl_write(cfg, 0);
		SWI2C_Delay();

		scl_write(cfg, 1);  // 产生时钟上升沿
        SWI2C_Delay();

		res <<= 1;
		if (sda_read(cfg)) { res |= 0x01; }

		SWI2C_Delay();
	}

	scl_write(cfg, 0);  // 结束后拉低SCL

	if (ack) SWI2C_SendACK(cfg); else SWI2C_SendNACK(cfg);
	*byte = res;
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_WriteBytes(void *user_data, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
	if (user_data == NULL || data == NULL || len == 0) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_status_t ret;
	ret = SWI2C_Start(user_data);
	if (ret != I2C_BUS_OK) { return ret; }
	
	ret = SWI2C_WriteByte(user_data, (dev_addr << 1) | 0x00);
	if (ret != I2C_BUS_OK) { SWI2C_Stop(user_data); return ret; }
	
	for (uint16_t i = 0; i < len; i++) {
		ret = SWI2C_WriteByte(user_data, data[i]);
		if (ret != I2C_BUS_OK) { SWI2C_Stop(user_data); return ret; }
	}
	
	return SWI2C_Stop(user_data);
}

static i2c_bus_status_t SWI2C_ReadBytes(void *user_data, uint8_t dev_addr, uint8_t *data, uint16_t len) {
	if (user_data == NULL || data == NULL || len == 0) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_status_t ret;
	ret = SWI2C_Start(user_data);
	if (ret != I2C_BUS_OK) { return ret; }
	
	ret = SWI2C_WriteByte(user_data, (dev_addr << 1) | 0x01);
	if (ret != I2C_BUS_OK) { SWI2C_Stop(user_data); return ret; }
	
	for (uint16_t i = 0; i < len; i++) {
		ret = SWI2C_ReadByte(user_data, &data[i], (i < len - 1));
		if (ret != I2C_BUS_OK) { SWI2C_Stop(user_data); return ret; }
	}
	
	return SWI2C_Stop(user_data);
}

static i2c_bus_ops_t SWI2C_OPS = {
	.init = SWI2C_Init,
    .start = SWI2C_Start,
    .stop = SWI2C_Stop,
    .write_byte = SWI2C_WriteByte,
    .read_byte = SWI2C_ReadByte,
	.write_bytes = SWI2C_WriteBytes,
	.read_bytes = SWI2C_ReadBytes
};

i2c_bus_status_t i2c_bus_stm32_std_lib_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_lib_sw_config_t *cfg) {
    if (handle == NULL || cfg == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	handle->user_data = cfg;
	handle->ops = &SWI2C_OPS;
	return handle->ops->init(handle->user_data);
}
