#include "Serial.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define SERIAL_USART			USART1
#define SERIAL_USART_CLK		RCC_APB2Periph_USART1
#define SERIAL_GPIO_CLK			RCC_APB2Periph_GPIOA
#define SERIAL_TX_GPIO_PIN		GPIO_Pin_9
#define SERIAL_RX_GPIO_PIN		GPIO_Pin_10
#define SERIAL_GPIO_PORT		GPIOA
#define SERIAL_BAUDRATE			115200

//int fputc(int ch, FILE *f) {
//	Serial_SendByte(ch);
//	return ch;
//}

static volatile bool Serial_InitFlag = false;

void Serial_Init(uint32_t BaudRate) {
	if (Serial_InitFlag) { return; }
	if (BaudRate == 0) { BaudRate = SERIAL_BAUDRATE; }
	RCC_APB2PeriphClockCmd(SERIAL_USART_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SERIAL_GPIO_CLK, ENABLE);
	// GPIO 引脚初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// TX 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SERIAL_TX_GPIO_PIN;
	GPIO_Init(SERIAL_GPIO_PORT, &GPIO_InitStructure);
	// RX 引脚初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = SERIAL_RX_GPIO_PIN;
	GPIO_Init(SERIAL_GPIO_PORT, &GPIO_InitStructure);
	// USART 初始化
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(SERIAL_USART, &USART_InitStructure);
	// USART 使能
	USART_Cmd(SERIAL_USART, ENABLE);
	Serial_InitFlag = true;
}

void Serial_SendByte(uint8_t byte) {
	if (!Serial_InitFlag) { return; }
	USART_SendData(SERIAL_USART, byte);
	while(USART_GetFlagStatus(SERIAL_USART, USART_FLAG_TXE) == RESET);
}

void Serial_Printf(const char *fmt, ...) {
	if (!Serial_InitFlag) { return; }
	char buf[255];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
//	if(len >= sizeof(buf)) {
//		return;
//	}
	// 通过 Serial_SendByte 逐字节发送
	for (int i = 0; buf[i] != '\0'; i++) {
		Serial_SendByte(buf[i]);
	}
}
