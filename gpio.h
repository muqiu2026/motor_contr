/*
 * gpio.h
 * GPIO配置头文件
 */

#ifndef GPIO_H
#define GPIO_H

#include "py32f0xx.h"

// LED引脚定义
#define LED_Pin                  GPIO_PIN_0
#define LED_GPIO_Port            GPIOA

// 按键引脚定义
#define KEY_Pin                  GPIO_PIN_1
#define KEY_GPIO_Port            GPIOA
#define KEY_EXTI_Line            EXTI_Line1

// 编码器引脚定义
#define ENCODER_A_Pin            GPIO_PIN_2
#define ENCODER_A_GPIO_Port      GPIOA
#define ENCODER_A_EXTI_Line      EXTI_Line2

#define ENCODER_B_Pin            GPIO_PIN_3
#define ENCODER_B_GPIO_Port      GPIOA
#define ENCODER_B_EXTI_Line      EXTI_Line3

// 电机PWM引脚定义
#define MOTOR_PWM1_Pin           GPIO_PIN_4
#define MOTOR_PWM1_GPIO_Port     GPIOA

#define MOTOR_PWM2_Pin           GPIO_PIN_5
#define MOTOR_PWM2_GPIO_Port     GPIOA

// 函数声明
void GPIO_Init(void);

#endif /* GPIO_H */
