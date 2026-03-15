/*
 * motor_control.c
 * 电机控制主文件
 * 使用PY32F002芯片控制有刷电机
 * 功能：
 * 1. 编码器检测电机运行方向和速度
 * 2. 双PWM控制电机
 * 3. LED指示灯
 * 4. 按键控制（正转/反转切换）
 * 5. 缓起缓停功能
 * 6. 记录电机转动圈数和运行方向
 * 7. 串口控制功能
 */

#include "py32f0xx.h"
#include "motor_control.h"
#include "encoder.h"
#include "pwm.h"
#include "usart.h"
#include "gpio.h"

// 全局变量
volatile uint32_t encoder_count = 0;  // 编码器计数
volatile uint8_t motor_direction = MOTOR_DIR_STOP;  // 电机运行方向
volatile uint16_t motor_target_speed = 0;  // 目标速度
volatile uint16_t motor_current_speed = 0;  // 当前速度
volatile uint32_t motor_position = 0;  // 电机位置（圈数）
volatile uint8_t key_state = 0;  // 按键状态
volatile uint8_t key_press = 0;  // 按键按下标志

// 电机控制状态机
typedef enum {
    MOTOR_STATE_STOP = 0,
    MOTOR_STATE_ACCELERATE,
    MOTOR_STATE_RUNNING,
    MOTOR_STATE_DECELERATE
} MotorState;

volatile MotorState motor_state = MOTOR_STATE_STOP;

// 电机控制参数
#define ACCELERATION_STEP 10  // 加速度步长
#define DECELERATION_STEP 15  // 减速度步长
#define MAX_SPEED 1000        // 最大速度

/**
 * @brief 系统初始化
 */
void SystemInit(void) {
    // 初始化系统时钟
    SystemClock_Config();
    
    // 初始化GPIO
    GPIO_Init();
    
    // 初始化PWM
    PWM_Init();
    
    // 初始化编码器
    Encoder_Init();
    
    // 初始化串口
    USART_Init();
    
    // 初始化定时器
    Timer_Init();
    
    // 启用中断
    __enable_irq();
}

/**
 * @brief 系统时钟配置
 */
void SystemClock_Config(void) {
    // 配置系统时钟为48MHz
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief 错误处理
 */
void Error_Handler(void) {
    while (1) {
        // 闪烁LED表示错误
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(500);
    }
}

/**
 * @brief 电机控制状态机处理
 */
void Motor_State_Process(void) {
    switch (motor_state) {
        case MOTOR_STATE_STOP:
            if (motor_target_speed > 0) {
                motor_state = MOTOR_STATE_ACCELERATE;
            }
            break;
            
        case MOTOR_STATE_ACCELERATE:
            if (motor_current_speed < motor_target_speed) {
                motor_current_speed += ACCELERATION_STEP;
                if (motor_current_speed > motor_target_speed) {
                    motor_current_speed = motor_target_speed;
                }
                Motor_Set_Speed(motor_current_speed, motor_direction);
            } else {
                motor_state = MOTOR_STATE_RUNNING;
            }
            break;
            
        case MOTOR_STATE_RUNNING:
            if (motor_current_speed != motor_target_speed) {
                if (motor_current_speed < motor_target_speed) {
                    motor_state = MOTOR_STATE_ACCELERATE;
                } else {
                    motor_state = MOTOR_STATE_DECELERATE;
                }
            }
            break;
            
        case MOTOR_STATE_DECELERATE:
            if (motor_current_speed > motor_target_speed) {
                motor_current_speed -= DECELERATION_STEP;
                if (motor_current_speed < motor_target_speed) {
                    motor_current_speed = motor_target_speed;
                }
                Motor_Set_Speed(motor_current_speed, motor_direction);
            } else {
                if (motor_target_speed == 0) {
                    motor_state = MOTOR_STATE_STOP;
                } else {
                    motor_state = MOTOR_STATE_RUNNING;
                }
            }
            break;
    }
}

/**
 * @brief 电机设置速度和方向
 * @param speed: 速度值 (0-1000)
 * @param direction: 方向 (MOTOR_DIR_FORWARD, MOTOR_DIR_BACKWARD, MOTOR_DIR_STOP)
 */
void Motor_Set_Speed(uint16_t speed, uint8_t direction) {
    if (direction == MOTOR_DIR_FORWARD) {
        // 正转
        PWM_Set_Duty(PWM_CHANNEL1, speed);
        PWM_Set_Duty(PWM_CHANNEL2, 0);
    } else if (direction == MOTOR_DIR_BACKWARD) {
        // 反转
        PWM_Set_Duty(PWM_CHANNEL1, 0);
        PWM_Set_Duty(PWM_CHANNEL2, speed);
    } else {
        // 停止
        PWM_Set_Duty(PWM_CHANNEL1, 0);
        PWM_Set_Duty(PWM_CHANNEL2, 0);
    }
}

/**
 * @brief 按键处理
 */
void Key_Process(void) {
    if (key_press) {
        key_press = 0;
        
        if (motor_direction == MOTOR_DIR_STOP || motor_direction == MOTOR_DIR_BACKWARD) {
            // 当前停止或反转，切换到正转
            // 先停止
            motor_target_speed = 0;
            while (motor_state != MOTOR_STATE_STOP) {
                Motor_State_Process();
                HAL_Delay(10);
            }
            // 再正转
            motor_direction = MOTOR_DIR_FORWARD;
            motor_target_speed = MAX_SPEED;
        } else if (motor_direction == MOTOR_DIR_FORWARD) {
            // 当前正转，切换到反转
            // 先停止
            motor_target_speed = 0;
            while (motor_state != MOTOR_STATE_STOP) {
                Motor_State_Process();
                HAL_Delay(10);
            }
            // 再反转
            motor_direction = MOTOR_DIR_BACKWARD;
            motor_target_speed = MAX_SPEED;
        }
        
        // 控制LED状态
        if (motor_direction != MOTOR_DIR_STOP) {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }
    }
}

/**
 * @brief 编码器计数处理
 */
void Encoder_Process(void) {
    static int32_t last_count = 0;
    int32_t current_count = encoder_count;
    int32_t delta = current_count - last_count;
    
    // 更新电机位置
    if (delta != 0) {
        motor_position += delta;
        
        // 更新运行方向
        if (delta > 0) {
            motor_direction = MOTOR_DIR_FORWARD;
        } else if (delta < 0) {
            motor_direction = MOTOR_DIR_BACKWARD;
        }
        
        last_count = current_count;
    }
}

/**
 * @brief 串口命令处理
 * @param cmd: 命令字符串
 */
void USART_Command_Process(uint8_t *cmd) {
    // 解析命令
    // 命令格式: CMD:参数1,参数2,...
    if (strstr((char*)cmd, "FORWARD") != NULL) {
        // 正转命令
        motor_target_speed = MAX_SPEED;
        motor_direction = MOTOR_DIR_FORWARD;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        USART_Send_String("Motor forward\r\n");
    } else if (strstr((char*)cmd, "BACKWARD") != NULL) {
        // 反转命令
        motor_target_speed = MAX_SPEED;
        motor_direction = MOTOR_DIR_BACKWARD;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        USART_Send_String("Motor backward\r\n");
    } else if (strstr((char*)cmd, "STOP") != NULL) {
        // 停止命令
        motor_target_speed = 0;
        motor_direction = MOTOR_DIR_STOP;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        USART_Send_String("Motor stop\r\n");
    } else if (strstr((char*)cmd, "POSITION") != NULL) {
        // 读取位置命令
        char buffer[64];
        sprintf(buffer, "Motor position: %ld\r\n", motor_position);
        USART_Send_String(buffer);
    } else if (strstr((char*)cmd, "DIRECTION") != NULL) {
        // 读取方向命令
        char buffer[64];
        if (motor_direction == MOTOR_DIR_FORWARD) {
            sprintf(buffer, "Motor direction: forward\r\n");
        } else if (motor_direction == MOTOR_DIR_BACKWARD) {
            sprintf(buffer, "Motor direction: backward\r\n");
        } else {
            sprintf(buffer, "Motor direction: stop\r\n");
        }
        USART_Send_String(buffer);
    } else if (strstr((char*)cmd, "RUN") != NULL) {
        // 运行指定圈数命令
        // 格式: RUN:圈数,方向
        uint32_t circles = 0;
        uint8_t dir = 0;
        sscanf((char*)cmd, "RUN:%ld,%d", &circles, &dir);
        
        // 计算目标位置
        int32_t target_position = motor_position + circles * ENCODER_PPR;
        
        // 设置方向
        if (dir == 1) {
            motor_direction = MOTOR_DIR_FORWARD;
        } else if (dir == -1) {
            motor_direction = MOTOR_DIR_BACKWARD;
        } else {
            USART_Send_String("Invalid direction\r\n");
            return;
        }
        
        // 开始运行
        motor_target_speed = MAX_SPEED;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        
        // 等待到达目标位置
        while ((motor_direction == MOTOR_DIR_FORWARD && motor_position < target_position) ||
               (motor_direction == MOTOR_DIR_BACKWARD && motor_position > target_position)) {
            Motor_State_Process();
            Encoder_Process();
            HAL_Delay(10);
        }
        
        // 停止
        motor_target_speed = 0;
        while (motor_state != MOTOR_STATE_STOP) {
            Motor_State_Process();
            HAL_Delay(10);
        }
        motor_direction = MOTOR_DIR_STOP;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        
        USART_Send_String("Motor run completed\r\n");
    } else {
        // 未知命令
        USART_Send_String("Unknown command\r\n");
    }
}

/**
 * @brief 主函数
 */
int main(void) {
    // 系统初始化
    SystemInit();
    
    // 初始化变量
    encoder_count = 0;
    motor_direction = MOTOR_DIR_STOP;
    motor_target_speed = 0;
    motor_current_speed = 0;
    motor_position = 0;
    key_state = 0;
    key_press = 0;
    motor_state = MOTOR_STATE_STOP;
    
    // 发送初始化完成消息
    USART_Send_String("Motor control system initialized\r\n");
    USART_Send_String("Commands: FORWARD, BACKWARD, STOP, POSITION, DIRECTION, RUN:circles,direction\r\n");
    
    // 主循环
    while (1) {
        // 按键处理
        Key_Process();
        
        // 编码器处理
        Encoder_Process();
        
        // 电机状态处理
        Motor_State_Process();
        
        // 延时
        HAL_Delay(10);
    }
}

/**
 * @brief 定时器中断处理函数
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        // 清除中断标志
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        
        // 在这里可以添加定时任务
    }
}

/**
 * @brief 外部中断处理函数
 */
void EXTI0_1_IRQHandler(void) {
    if (EXTI_GetITStatus(KEY_EXTI_Line) != RESET) {
        // 清除中断标志
        EXTI_ClearITPendingBit(KEY_EXTI_Line);
        
        // 按键防抖
        HAL_Delay(10);
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
            key_press = 1;
        }
    }
}

/**
 * @brief 编码器中断处理函数
 */
void EXTI2_3_IRQHandler(void) {
    if (EXTI_GetITStatus(ENCODER_A_EXTI_Line) != RESET || EXTI_GetITStatus(ENCODER_B_EXTI_Line) != RESET) {
        // 清除中断标志
        EXTI_ClearITPendingBit(ENCODER_A_EXTI_Line);
        EXTI_ClearITPendingBit(ENCODER_B_EXTI_Line);
        
        // 读取编码器状态
        uint8_t a_state = HAL_GPIO_ReadPin(ENCODER_A_GPIO_Port, ENCODER_A_Pin);
        uint8_t b_state = HAL_GPIO_ReadPin(ENCODER_B_GPIO_Port, ENCODER_B_Pin);
        
        // 计算编码器计数
        if (a_state != b_state) {
            encoder_count++;
        } else {
            encoder_count--;
        }
    }
}
