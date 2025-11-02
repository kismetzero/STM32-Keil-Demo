#include "USART1.h"

#define USART1_GPIO_PORT		GPIOA
#define USART1_TX_GPIO_PIN		GPIO_Pin_9
#define USART1_RX_GPIO_PIN		GPIO_Pin_10
#define USART1_GPIO_CLK			RCC_APB2Periph_GPIOA
#define USART1_BaudRate			115200

//int fputc(int ch, FILE *f) {
//	USART1_SendByte(ch);
//	return ch;
//}

void USART1_Init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	RCC_APB2PeriphClockCmd(USART1_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = USART1_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = USART1_RX_GPIO_PIN;
	GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = USART1_BaudRate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

uint8_t USART1_SendByte(uint8_t Byte) {
	USART_SendData(USART1, Byte);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	return Byte;
}

void USART1_printf(const char *format, ...) {
	char buffer[255];
	va_list args;     // 定义可变参数列表

    // 将可变参数列表格式化到 buffer 中
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
	
	// 通过 USART1 逐字节发送
    for (int i = 0; buffer[i] != '\0'; i++) {
        USART1_SendByte(buffer[i]);
    }
}
