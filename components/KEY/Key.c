#include "Key.h"
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "axk_ssd1306.h"
#include "cmsis_os2.h"
#include "ina226.h"

extern osMessageQueueId_t BtnQueueHandle;

/**
 * @brief 检查按键状态并将按键编号发送到消息队列
 * 
 * 功能描述：
 * - 检查三个按键的状态（切换键、增加键、减小键）
 * - 消抖处理，确保按键稳定后再读取状态
 * - 将按键编号通过消息队列发送出去
 */
void KEY_NUM(void) {
    uint8_t btn_num = 0;

    // 检查按键 1 (切换键)
    if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
        osDelay(10); // 消抖处理
        if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
            btn_num = 1;
            osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0); 
            while (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
                osDelay(20); // 等待松开，20ms 检查一次即可，节省 CPU
            }
        }
    }

    // 检查按键 2 (增加键)
    if (HAL_GPIO_ReadPin(UpKey_GPIO_Port, UpKey_Pin) == GPIO_PIN_RESET) {
        osDelay(10);
        if (HAL_GPIO_ReadPin(UpKey_GPIO_Port, UpKey_Pin) == GPIO_PIN_RESET) {
            btn_num = 2;
            osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0);
            while (HAL_GPIO_ReadPin(UpKey_GPIO_Port, UpKey_Pin) == GPIO_PIN_RESET) {
                osDelay(20);
            }
        }
    }

    // 检查按键 3 (减小键)
    else if (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
        osDelay(10);
        if (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
            btn_num = 3;
            osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0);
            while (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
                osDelay(20);
            }
        }
    }
} 

/**
 * @brief 控制输出状态
 * 
 * @param num 输出控制编号：
 * - 0：关闭输出
 * - 1：开启输出
 * - 2：翻转输出状态
 */
void KEY_Output(uint8_t num) {
    switch (num) {
        case 0: // 设置关
            HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_RESET);
            break;    
        case 1: // 设置开
            HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_SET);
            break;
        case 2: // 翻转
            HAL_GPIO_TogglePin(OutputKey_GPIO_Port, OutputKey_Pin);
            break;
    }
}

/**
 * @brief 显示当前输出状态
 * 
 * 功能描述：
 * - 在 OLED 屏幕上显示当前输出状态（开/关）
 */
void KEY_state(void) {
    if (HAL_GPIO_ReadPin(OutputKey_GPIO_Port, OutputKey_Pin) == GPIO_PIN_SET) {
        axk_ssd1306_show_utf8_str(92, 6, "开");
    } else {
        axk_ssd1306_show_utf8_str(92, 6, "关");
    }
}

