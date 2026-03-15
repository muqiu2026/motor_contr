/*
 * motor_control.h
 * 电机控制头文件
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "py32f0xx.h"

// 电机方向定义
#define MOTOR_DIR_STOP     0  // 停止
#define MOTOR_DIR_FORWARD  1  // 正转
#define MOTOR_DIR_BACKWARD 2  // 反转

// 函数声明
void SystemInit(void);
void SystemClock_Config(void);
void Error_Handler(void);
void Motor_State_Process(void);
void Motor_Set_Speed(uint16_t speed, uint8_t direction);
void Key_Process(void);
void Encoder_Process(void);
void USART_Command_Process(uint8_t *cmd);

#endif /* MOTOR_CONTROL_H */
