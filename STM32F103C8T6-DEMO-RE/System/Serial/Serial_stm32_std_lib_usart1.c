#include "Serial.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>

#define SERIAL_USARTX			USART1
#define SERIAL_USARTX_CLK		RCC_APB2Periph_USART1
#define SERIAL_GPIO_CLK			RCC_APB2Periph_GPIOA
#define SERIAL_TX_GPIO_PIN		GPIO_Pin_9
#define SERIAL_RX_GPIO_PIN		GPIO_Pin_10
#define SERIAL_GPIO_PORT		GPIOA
#define SERIAL_BaudRate			115200

//int fputc(int ch, FILE *f) {
//	SERIAL_SendByte(ch);
//	return ch;
//}

void Serial_Init(void) {
	RCC_APB2PeriphClockCmd(SERIAL_USARTX_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SERIAL_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SERIAL_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SERIAL_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = SERIAL_RX_GPIO_PIN;
	GPIO_Init(SERIAL_GPIO_PORT, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = SERIAL_BaudRate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(SERIAL_USARTX, &USART_InitStructure);
	
	USART_Cmd(SERIAL_USARTX, ENABLE);
}

uint8_t SERIAL_SendByte(uint8_t Byte) {
	USART_SendData(SERIAL_USARTX, Byte);
	while(USART_GetFlagStatus(SERIAL_USARTX, USART_FLAG_TXE) == RESET);
	return Byte;
}

void Serial_Printf(const char *format, ...) {
	char buffer[255];
	va_list args;		// 定义可变参数列表

	// 将可变参数列表格式化到 buffer 中
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	
	// 通过 SERIAL_SendByte 逐字节发送
	for (int i = 0; buffer[i] != '\0'; i++) {
		SERIAL_SendByte(buffer[i]);
	}
}

