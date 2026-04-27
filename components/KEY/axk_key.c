#include "axk_key.h"
#include "main.h"
#include <stdint.h>
#include "axk_ssd1306.h"
#include "cmsis_os2.h"
#include "axk_ina226.h"

extern osMessageQueueId_t BtnQueueHandle;


/*
 * axk_key_scan：检查按键状态并发送按键编号到消息队列
 * @brief  检查三个按键状态，消抖处理后通过消息队列发送按键编号
 * @param  无
 * @return 无
 * @note   消抖时间 10ms，等待松开间隔 20ms
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_key_scan(void)
{
    uint8_t btn_num = 0;

    if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
        osDelay(10);

        if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
            btn_num = 1;
            osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0);

            while (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
                osDelay(20);
            }
        }
    }

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

    if (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
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


/*
 * axk_key_output：控制输出状态
 * @brief  通过GPIO控制设备输出开关
 * @param  num：输出控制编号，0：关闭，1：开启，2：翻转
 * @return 无
 * @note   参数非法时直接返回
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_key_output(uint8_t num)
{
    if (num > 2) {
        return;
    }

    switch (num) {
        case 0:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            break;
        case 1:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
            break;
    }
}


/*
 * axk_key_state：显示当前输出状态
 * @brief  在 OLED 屏幕上显示当前输出状态（开/关）
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_key_state(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) {
        axk_ssd1306_show_utf8_str(72, 6, "开");
    } else {
        axk_ssd1306_show_utf8_str(72, 6, "关");
    }
}
