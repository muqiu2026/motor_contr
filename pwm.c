/*
 * pwm.c
 * PWM配置实现文件
 */

#include "pwm.h"

/**
 * @brief PWM初始化
 */
void PWM_Init(void) {
    TIM_HandleTypeDef htim1;
    TIM_OC_InitTypeDef sConfigOC;

    // 启用TIM1时钟
    __HAL_RCC_TIM1_CLK_ENABLE();

    // 配置TIM1
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 47;  // 48MHz / (47+1) = 1MHz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = PWM_PERIOD - 1;  // 1kHz
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    // 配置通道1
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    // 配置通道2
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }

    // 启动PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
}

/**
 * @brief 设置PWM占空比
 * @param channel: 通道号 (PWM_CHANNEL1, PWM_CHANNEL2)
 * @param duty: 占空比 (0-1000)
 */
void PWM_Set_Duty(uint8_t channel, uint16_t duty) {
    // 确保占空比在有效范围内
    if (duty > PWM_PERIOD) {
        duty = PWM_PERIOD;
    }

    // 设置占空比
    if (channel == PWM_CHANNEL1) {
        TIM1->CCR1 = duty;
    } else if (channel == PWM_CHANNEL2) {
        TIM1->CCR2 = duty;
    }
}
