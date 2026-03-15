/*
 * usart.h
 * 串口配置头文件
 */

#ifndef USART_H
#define USART_H

#include "py32f0xx.h"

// 串口参数
#define USART_BAUDRATE   115200  // 波特率

// 函数声明
void USART_Init(void);
void USART_Send_String(uint8_t *str);
void USART_Receive_String(uint8_t *str, uint16_t length);

#endif /* USART_H */
