/*
 * usart.c
 * 串口配置实现文件
 */

#include "usart.h"

// 全局变量
volatile uint8_t usart_rx_buffer[256];
volatile uint16_t usart_rx_index = 0;

/**
 * @brief 串口初始化
 */
void USART_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    USART_InitTypeDef USART_InitStruct = {0};
    NVIC_InitTypeDef NVIC_InitStruct = {0};

    // 启用GPIOA和USART1时钟
    RCC_GPIOA_CLK_ENABLE();
    RCC_USART1_CLK_ENABLE();

    // 配置TX引脚
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置RX引脚
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置USART1
    USART_InitStruct.BaudRate = USART_BAUDRATE;
    USART_InitStruct.WordLength = USART_WORDLENGTH_8B;
    USART_InitStruct.StopBits = USART_STOPBITS_1;
    USART_InitStruct.Parity = USART_PARITY_NONE;
    USART_InitStruct.Mode = USART_MODE_TX_RX;
    USART_InitStruct.CLKPolarity = USART_POLARITY_LOW;
    USART_InitStruct.CLKPhase = USART_PHASE_1EDGE;
    USART_InitStruct.CLKLastBit = USART_LASTBIT_DISABLE;
    HAL_USART_Init(&huart1, &USART_InitStruct);

    // 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 启用接收中断
    __HAL_USART_ENABLE_IT(&huart1, USART_IT_RXNE);
}

/**
 * @brief 发送字符串
 * @param str: 字符串指针
 */
void USART_Send_String(uint8_t *str) {
    while (*str) {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *str++;
    }
}

/**
 * @brief 接收字符串
 * @param str: 字符串指针
 * @param length: 字符串长度
 */
void USART_Receive_String(uint8_t *str, uint16_t length) {
    uint16_t i = 0;
    while (i < length) {
        while (!(USART1->ISR & USART_ISR_RXNE));
        str[i++] = USART1->RDR;
    }
}

/**
 * @brief USART1中断处理函数
 */
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // 读取接收数据
        uint8_t data = USART_ReceiveData(USART1);
        
        // 存储到缓冲区
        if (usart_rx_index < 255) {
            usart_rx_buffer[usart_rx_index++] = data;
            
            // 检测回车换行符
            if (data == '\n' || data == '\r') {
                usart_rx_buffer[usart_rx_index] = '\0';
                usart_rx_index = 0;
                
                // 处理命令
                USART_Command_Process(usart_rx_buffer);
            }
        } else {
            // 缓冲区溢出，重置索引
            usart_rx_index = 0;
        }
        
        // 清除中断标志
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
