/*
 * encoder.h
 * 编码器配置头文件
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "py32f0xx.h"

// 编码器参数
#define ENCODER_PPR      200  // 编码器每转脉冲数

// 函数声明
void Encoder_Init(void);

#endif /* ENCODER_H */
