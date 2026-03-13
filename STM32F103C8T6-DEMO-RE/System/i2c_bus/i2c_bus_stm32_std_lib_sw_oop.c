#include "i2c_bus_stm32_std_lib_sw.h"
#include "delay.h"
#include "USART1.h"

static inline void SWI2C_Delay(void) {
	delay_us(5);
}

static inline uint8_t scl_read(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->scl_gpio_port, cfg->scl_gpio_pin);
}

static inline void scl_write(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->scl_gpio_port, cfg->scl_gpio_pin, val ? Bit_SET : Bit_RESET);
}

static inline uint8_t sda_read(i2c_bus_stm32_std_lib_sw_config_t *cfg) {
	return GPIO_ReadInputDataBit(cfg->sda_gpio_port, cfg->sda_gpio_pin);
}

static inline void sda_write(i2c_bus_stm32_std_lib_sw_config_t *cfg, uint8_t val) {
	GPIO_WriteBit(cfg->sda_gpio_port, cfg->sda_gpio_pin, val ? Bit_SET : Bit_RESET);
}

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
	// SDA 下降沿
	sda_write(cfg, 0);
	SWI2C_Delay();
	// 拉低 SCL，准备发送数据
	scl_write(cfg, 0);	
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
	// 先释放 SCL
	scl_write(cfg, 1);
	SWI2C_Delay();
	// SDA 上升沿
	sda_write(cfg, 1);
	SWI2C_Delay();
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_SendACK(void *user_data, bool ack) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	
	scl_write(cfg, 0);
	if (ack) { sda_write(cfg, 0); }	// SDA = 0 表示应答
	else { sda_write(cfg, 1); }		// SDA = 1 表示非应答
	SWI2C_Delay();
	// 时钟高电平，从设备采样ACK
	scl_write(cfg, 1);
	SWI2C_Delay();

	scl_write(cfg, 0);
	sda_write(cfg, 1);
	SWI2C_Delay();
	
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_WaitACK(void *user_data) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	uint8_t timeout = 0;
	// 释放SDA，让从机拉低
	scl_write(cfg, 0);
	sda_write(cfg, 1);
	SWI2C_Delay();
	// 主机释放SCL，等待从机拉低SDA
	scl_write(cfg, 1);
	SWI2C_Delay();
	// 循环等待
	while (sda_read(cfg) && timeout < 255) {
		timeout++;
		SWI2C_Delay();
    }
	// 拉低时钟
	scl_write(cfg, 0);
	SWI2C_Delay();
	// 超时，无ACK
	if (timeout >= 255) { return I2C_BUS_ERR_NACK; }
	return I2C_BUS_OK;
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
		// 产生时钟上升沿
		scl_write(cfg, 1);
        SWI2C_Delay();

		res <<= 1;
		if (sda_read(cfg)) { res |= 0x01; }

		SWI2C_Delay();
	}
	
	scl_write(cfg, 0);  // 结束后拉低SCL
	
	i2c_bus_status_t ret;
	ret = SWI2C_SendACK(user_data, ack);
	if (ret != I2C_BUS_OK) { return ret; }
	*byte = res;
	
	return I2C_BUS_OK;
}

static i2c_bus_status_t SWI2C_WriteByte(void *user_data, uint8_t byte, bool wait) {
	if (user_data == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	i2c_bus_stm32_std_lib_sw_config_t *cfg = (i2c_bus_stm32_std_lib_sw_config_t *)user_data;
	for (uint8_t i = 0; i < 8; i++) {
		// 拉低时钟
		scl_write(cfg, 0);
		SWI2C_Delay();

		if (byte & 0x80) { sda_write(cfg, 1); }
		else { sda_write(cfg, 0); }
		byte <<= 1;
		SWI2C_Delay();

		scl_write(cfg, 1);
		SWI2C_Delay();
	}
	scl_write(cfg, 0);
	if (wait) { return SWI2C_WaitACK(user_data); }
	return I2C_BUS_OK;
}

static i2c_bus_ops_t SWI2C_OPS = {
	.init = SWI2C_Init,
    .start = SWI2C_Start,
    .stop = SWI2C_Stop,
	.send_ack = SWI2C_SendACK,
	.wait_ack = SWI2C_WaitACK,
	.read_byte = SWI2C_ReadByte,
    .write_byte = SWI2C_WriteByte
};

i2c_bus_status_t i2c_bus_stm32_std_lib_sw_create_handle(i2c_bus_handle_t *handle, i2c_bus_stm32_std_lib_sw_config_t *cfg) {
    if (handle == NULL || cfg == NULL) { return I2C_BUS_ERR_INVALID_PARAM; }
	handle->user_data = cfg;
	handle->ops = &SWI2C_OPS;
	return handle->ops->init(handle->user_data);
}
