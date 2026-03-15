/*
 * timer.c
 * 定时器配置实现文件
 */

#include "timer.h"

/**
 * @brief 定时器初始化
 */
void Timer_Init(void) {
    TIM_HandleTypeDef htim1;
    NVIC_InitTypeDef NVIC_InitStruct = {0};

    // 启用TIM1时钟
    __HAL_RCC_TIM1_CLK_ENABLE();

    // 配置TIM1
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 47999;  // 48MHz / (47999+1) = 1kHz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 999;  // 1ms
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    // 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 启用更新中断
    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

    // 启动定时器
    HAL_TIM_Base_Start(&htim1);
}
