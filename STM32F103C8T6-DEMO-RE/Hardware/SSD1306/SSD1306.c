#include "SSD1306.h"
#include "SWI2C.h"

// SSD1306 I2C 地址（7位地址，通常为 0x3C 或 0x3D）
#define SSD1306_I2C_ADDR    0x3C	// 0011 1100

// 控制字节
#define SSD1306_CMD_MODE    0x00
#define SSD1306_DATA_MODE   0x40

void SSD1306_SendCommand(uint8_t command) {
    SWI2C_Start();
    SWI2C_WriteByte(SSD1306_I2C_ADDR << 1);  // 写地址
    if (SWI2C_WaitACK()) {
        SWI2C_Stop();
        return;
    }

    SWI2C_WriteByte(SSD1306_CMD_MODE);       // 控制字节：命令模式
    SWI2C_WaitACK();

    SWI2C_WriteByte(command);                // 发送命令
    SWI2C_WaitACK();

    SWI2C_Stop();
}

void SSD1306_SendCommands(uint8_t* commands, uint8_t count) {
    SWI2C_Start();
    SWI2C_WriteByte(SSD1306_I2C_ADDR << 1);
    if (SWI2C_WaitACK()) {
        SWI2C_Stop();
        return;
    }

    SWI2C_WriteByte(SSD1306_CMD_MODE);
    SWI2C_WaitACK();

    for (uint8_t i = 0; i < count; i++) {
        SWI2C_WriteByte(commands[i]);
        SWI2C_WaitACK();
    }

    SWI2C_Stop();
}

void SSD1306_SendData(uint8_t* Data, uint8_t count) {
    SWI2C_Start();
    SWI2C_WriteByte(SSD1306_I2C_ADDR << 1);
    if (SWI2C_WaitACK()) {
        SWI2C_Stop();
        return;
    }

    SWI2C_WriteByte(SSD1306_DATA_MODE);
    SWI2C_WaitACK();

    for (uint8_t i = 0; i < count; i++) {
        SWI2C_WriteByte(Data[i]);
        SWI2C_WaitACK();
    }

    SWI2C_Stop();
}

void SSD1306_Init(void) {
	SWI2C_Init();
	
	/*写入一系列的命令，对OLED进行初始化配置*/
	SSD1306_SendCommand(0xAE);	//设置显示开启/关闭，0xAE关闭，0xAF开启
	
	SSD1306_SendCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	SSD1306_SendCommand(0x80);	//0x00~0xFF
	
	SSD1306_SendCommand(0xA8);	//设置多路复用率
	SSD1306_SendCommand(0x3F);	//0x0E~0x3F
	
	SSD1306_SendCommand(0xD3);	//设置显示偏移
	SSD1306_SendCommand(0x00);	//0x00~0x7F
	
	SSD1306_SendCommand(0x40);	//设置显示开始行，0x40~0x7F
	
	SSD1306_SendCommand(0xA1);	//设置左右方向，0xA1正常，0xA0左右反置
	
	SSD1306_SendCommand(0xC8);	//设置上下方向，0xC8正常，0xC0上下反置

	SSD1306_SendCommand(0xDA);	//设置COM引脚硬件配置
	SSD1306_SendCommand(0x12);
	
	SSD1306_SendCommand(0x81);	//设置对比度
	SSD1306_SendCommand(0xCF);	//0x00~0xFF

	SSD1306_SendCommand(0xD9);	//设置预充电周期
	SSD1306_SendCommand(0xF1);

	SSD1306_SendCommand(0xDB);	//设置VCOMH取消选择级别
	SSD1306_SendCommand(0x30);

	SSD1306_SendCommand(0xA4);	//设置整个显示打开/关闭

	SSD1306_SendCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

	SSD1306_SendCommand(0x8D);	//设置充电泵
	SSD1306_SendCommand(0x14);

	SSD1306_SendCommand(0xAF);	//开启显示
}

