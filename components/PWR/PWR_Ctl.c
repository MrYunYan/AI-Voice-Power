#include "PWR_Ctl.h"
#include "emMCP.h"
#include "cJSON.h"
#include "emMCPLOG.h"
#include "gpio.h"
#include "main.h"
#include "axk_ch224.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include "uartPort.h"
#include <stdio.h>

// 声明一个全局状态实例
SysCtrl_t sys_ctrl = {
    .target_v = 5.0f,     // 默认上电目标为5V
    .real_v = 0.0f,
    .is_output_on = 0,
    .is_pps = 0
};

/**
 * @brief 按键电压切换处理函数
 * @param key_V 按键编号（0-4）
 * @note 通过按键切换不同固定电压档位，退出PPS模式
 */
void handle_key_voltage_change(uint8_t key_V) {
    sys_ctrl.is_pps = 0; // 按键切换的是固定档位，退出PPS模式
    switch (key_V) {
        case 0:
            axk_ch224_set_vout(AXK_CH224_VOUT_5V);
            sys_ctrl.target_v = 5.0f;
            break;
        case 1:
            axk_ch224_set_vout(AXK_CH224_VOUT_9V);
            sys_ctrl.target_v = 9.0f;
            break;
        case 2:
            axk_ch224_set_vout(AXK_CH224_VOUT_12V);
            sys_ctrl.target_v = 12.0f;
            break;
        case 3:
            axk_ch224_set_vout(AXK_CH224_VOUT_15V);
            sys_ctrl.target_v = 15.0f;
            break;
        case 4:
            axk_ch224_set_vout(AXK_CH224_VOUT_20V);
            sys_ctrl.target_v = 20.0f;
            break;
    }
}

/**
 * @brief 串口PPS电压设置处理函数
 * @param pps_val PPS电压值
 * @note 通过串口命令设置PPS电压，进入PPS模式
 */
void handle_serial_pps_change(float pps_val) {
    if (axk_ch224_set_pps_vout(pps_val) == 0) { // 如果设置成功
        sys_ctrl.is_pps = 1;         // 进入PPS模式
        sys_ctrl.target_v = pps_val; // 更新目标电压
    }
}

/**
 * @brief （输出开关）控制回调函数
 * @param arg JSON对象，包含power_state字段
 * @note 通过JSON命令控制输出开关，并返回操作结果
 */
void emMCP_SetRelayHandler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *led_state = cJSON_GetObjectItem(param, "power_state");
    if (led_state != NULL) {
        if (led_state->valueint == true) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            sys_ctrl.is_output_on = 1;
            emMCP_ResponseValue("{\"power_state\":true}");
        } else if (led_state->valueint == false) {
            sys_ctrl.is_output_on = 0;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            emMCP_ResponseValue("{\"power_state\":false}");
        }
    }
}

/**
 * @brief 查询（输出开关）状态回调函数
 * @param arg 未使用参数
 * @note 读取GPIO状态并返回JSON格式的开关状态
 */
void emMCP_GetRelayHandler(void *arg)
{
    char response_str[64];
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) {
        sprintf(response_str, "{\"power_state\":true}");
    } else {
        sprintf(response_str, "{\"power_state\":false}");
    }
    emMCP_ResponseValue(response_str);
}

/**
 * @brief 设置输出电压回调函数
 * @param arg JSON对象，包含voltage_value字段
 * @note 支持5.0V-20.0V，精度0.1V，设置成功后进入PPS模式
 */
void emMCP_SetVoltageHandler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *voltage_value = cJSON_GetObjectItem(param, "voltage_value");
    if (voltage_value != NULL) {
        float target_voltage = (float)voltage_value->valuedouble;
        if (target_voltage >= 5.0f && target_voltage <= 20.0f) {
            if (axk_ch224_set_mode(AXK_CH224_VOUT_PPS) == 0) {
                if (axk_ch224_set_pps_vout(target_voltage) == 0) {
                    sys_ctrl.is_pps = 1;
                    sys_ctrl.target_v = target_voltage;
                    int voltage_int = (int)target_voltage;
                    int voltage_frac = (int)((target_voltage - voltage_int) * 10);
                    if (voltage_frac < 0) voltage_frac = -voltage_frac;
                    char response_str[64];
                    sprintf(response_str, "{\"voltage_value\":\"%d.%01d\"}", voltage_int, voltage_frac);
                    emMCP_ResponseValue(response_str);
                }
            }
        }
    } 
}

/**
 * @brief 查询输出电压回调函数
 * @param arg 未使用参数
 * @note 输出关闭时返回目标电压，开启时返回实际电压
 */
void emMCP_GetVoltageHandler(void *arg)
{
    char response_str[64];
    float voltage_to_report;
    if (sys_ctrl.is_output_on == 0) {
        voltage_to_report = sys_ctrl.target_v;
    } else {
        voltage_to_report = sys_ctrl.real_v;
    }
    int voltage_int = (int)voltage_to_report;
    int voltage_frac = (int)((voltage_to_report - voltage_int) * 10);
    if (voltage_frac < 0) voltage_frac = -voltage_frac;
    sprintf(response_str, "{\"voltage_value\":\"%d.%01d\"}", voltage_int, voltage_frac);
    emMCP_ResponseValue(response_str);
}

/**
 * @brief 查询设备输出参数（电流、功率）回调函数
 * @param arg JSON对象，包含current_value和power_value字段
 * @note 通过INA226读取电流和功率，同时返回两个参数的值
 *       电流精度0.001A，功率精度0.001W
 */
void emMCP_GetOutputParamsHandler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *current_item = cJSON_GetObjectItem(param, "current_value");
    cJSON *power_item = cJSON_GetObjectItem(param, "power_value");
    
    float current = INA226_GetCurrent(&my_power_monitor);
    float power = INA226_GetPower(&my_power_monitor);
    
    // 处理电流值（保留3位小数）
    int current_int = (int)current;
    int current_frac = (int)((current - current_int) * 1000);
    if (current_frac < 0) current_frac = -current_frac;
    
    // 处理功率值（保留3位小数）
    int power_int = (int)power;
    int power_frac = (int)((power - power_int) * 1000);
    if (power_frac < 0) power_frac = -power_frac;
    
    // 构建响应JSON，同时返回电流和功率
    char response_str[128];
    sprintf(response_str, "{\"current_value\":\"%d.%03d\",\"power_value\":\"%d.%03d\"}", 
            current_int, current_frac, power_int, power_frac);
    emMCP_ResponseValue(response_str);
}
