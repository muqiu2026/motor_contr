/*
 * pwm.h
 * PWM配置头文件
 */

#ifndef PWM_H
#define PWM_H

#include "py32f0xx.h"

// PWM通道定义
#define PWM_CHANNEL1     0  // 通道1
#define PWM_CHANNEL2     1  // 通道2

// PWM参数
#define PWM_PERIOD       1000  // PWM周期

// 函数声明
void PWM_Init(void);
void PWM_Set_Duty(uint8_t channel, uint16_t duty);

#endif /* PWM_H */
