#include "axk_pwr_ctl.h"
#include <stdio.h>
#include "emMCP.h"
#include "cJSON.h"
#include "emMCPLOG.h"
#include "axk_ch224.h"
#include "main.h"
#include "gpio.h"
#include "uartPort.h"

AxkSysCtrl g_axk_sys_ctrl = {
    .target_v = 5.0f,
    .real_v = 0.0f,
    .is_output_on = 0,
    .is_pps = 0
};

AxkIna226Device g_axk_power_monitor;


/*
 * axk_pwr_handle_key_voltage：按键电压切换处理
 * @brief  通过按键切换不同固定电压档位，退出PPS模式
 * @param  key_v：按键编号，取值范围：0-4
 * @return 无
 * @note   按键切换的是固定档位，会自动退出PPS模式
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_handle_key_voltage(uint8_t key_v)
{
    if (key_v > 4) {
        return;
    }

    g_axk_sys_ctrl.is_pps = 0;

    switch (key_v) {
        case 0:
            axk_ch224_set_vout(AXK_CH224_VOUT_5V);
            g_axk_sys_ctrl.target_v = 5.0f;
            break;
        case 1:
            axk_ch224_set_vout(AXK_CH224_VOUT_9V);
            g_axk_sys_ctrl.target_v = 9.0f;
            break;
        case 2:
            axk_ch224_set_vout(AXK_CH224_VOUT_12V);
            g_axk_sys_ctrl.target_v = 12.0f;
            break;
        case 3:
            axk_ch224_set_vout(AXK_CH224_VOUT_15V);
            g_axk_sys_ctrl.target_v = 15.0f;
            break;
        case 4:
            axk_ch224_set_vout(AXK_CH224_VOUT_20V);
            g_axk_sys_ctrl.target_v = 20.0f;
            break;
    }
}


/*
 * axk_pwr_handle_serial_pps_change：串口PPS电压设置处理
 * @brief  通过串口命令设置PPS电压，进入PPS模式
 * @param  pps_val：PPS电压值
 * @return 无
 * @note   设置成功后才更新目标电压和PPS模式标志
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_handle_serial_pps_change(float pps_val)
{
    if (axk_ch224_set_pps_vout(pps_val) == 0) {
        g_axk_sys_ctrl.is_pps = 1;
        g_axk_sys_ctrl.target_v = pps_val;
    }
}


/*
 * axk_pwr_set_relay_handler：（输出开关）控制回调函数
 * @brief  通过JSON命令控制输出开关，并返回操作结果
 * @param  arg：JSON对象，包含power_state字段
 * @return 无
 * @note   emMCP工具回调函数；true开启输出，false关闭输出
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_set_relay_handler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *led_state = cJSON_GetObjectItem(param, "power_state");

    if (led_state != NULL) {
        if (led_state->valueint == true) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            g_axk_sys_ctrl.is_output_on = 1;
            emMCP_ResponseValue("{\"power_state\":true}");
        } else if (led_state->valueint == false) {
            g_axk_sys_ctrl.is_output_on = 0;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            emMCP_ResponseValue("{\"power_state\":false}");
        }
    }
}


/*
 * axk_pwr_get_relay_handler：查询（输出开关）状态回调函数
 * @brief  读取GPIO状态并返回JSON格式的开关状态
 * @param  arg：未使用参数
 * @return 无
 * @note   emMCP工具回调函数
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_get_relay_handler(void *arg)
{
    char response_str[64];

    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) {
        sprintf(response_str, "{\"power_state\":true}");
    } else {
        sprintf(response_str, "{\"power_state\":false}");
    }

    emMCP_ResponseValue(response_str);
}


/*
 * axk_pwr_set_voltage_handler：设置输出电压回调函数
 * @brief  通过JSON命令设置输出电压，支持5.0V-20.0V，精度0.1V
 * @param  arg：JSON对象，包含voltage_value字段
 * @return 无
 * @note   emMCP工具回调函数；设置成功后进入PPS模式
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_set_voltage_handler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *voltage_value = cJSON_GetObjectItem(param, "voltage_value");

    if (voltage_value != NULL) {
        float target_voltage = (float)voltage_value->valuedouble;

        if (target_voltage >= 5.0f && target_voltage <= 20.0f) {
            if (axk_ch224_set_mode(AXK_CH224_VOUT_PPS) == 0) {
                if (axk_ch224_set_pps_vout(target_voltage) == 0) {
                    g_axk_sys_ctrl.is_pps = 1;
                    g_axk_sys_ctrl.target_v = target_voltage;

                    int voltage_decivolts = (int)(target_voltage * 10.0f + 0.5f);
                    int voltage_int = voltage_decivolts / 10;
                    int voltage_frac = voltage_decivolts % 10;
                    char response_str[64];

                    sprintf(response_str, "{\"voltage_value\":\"%d.%d\"}", voltage_int, voltage_frac);
                    emMCP_ResponseValue(response_str);
                }
            }
        }
    }
}


/*
 * axk_pwr_get_voltage_handler：查询输出电压回调函数
 * @brief  输出关闭时返回目标电压，开启时返回实际电压
 * @param  arg：未使用参数
 * @return 无
 * @note   emMCP工具回调函数
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_get_voltage_handler(void *arg)
{
    char response_str[64];
    float voltage_to_report;

    if (g_axk_sys_ctrl.is_output_on == 0) {
        voltage_to_report = g_axk_sys_ctrl.target_v;
    } else {
        voltage_to_report = g_axk_sys_ctrl.real_v;
    }

    int voltage_decivolts = (int)(voltage_to_report * 10.0f + 0.5f);
    int voltage_int = voltage_decivolts / 10;
    int voltage_frac = voltage_decivolts % 10;

    sprintf(response_str, "{\"voltage_value\":\"%d.%d\"}", voltage_int, voltage_frac);
    emMCP_ResponseValue(response_str);
}


/*
 * axk_pwr_get_output_params_handler：查询设备输出参数（电流、功率）回调函数
 * @brief  通过INA226读取电流和功率，同时返回两个参数的值
 * @param  arg：JSON对象，包含current_value和power_value字段
 * @return 无
 * @note   emMCP工具回调函数；电流精度0.001A，功率精度0.001W
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_pwr_get_output_params_handler(void *arg)
{
    (void)arg;

    float current = axk_ina226_get_current(&g_axk_power_monitor);
    float power = axk_ina226_get_power(&g_axk_power_monitor);

    int current_milliamps = (int)(current * 1000.0f + 0.5f);
    int current_int = current_milliamps / 1000;
    int current_frac = current_milliamps % 1000;

    if (current_frac < 0) {
        current_frac = -current_frac;
    }

    int power_milliwatts = (int)(power * 1000.0f + 0.5f);
    int power_int = power_milliwatts / 1000;
    int power_frac = power_milliwatts % 1000;

    if (power_frac < 0) {
        power_frac = -power_frac;
    }

    char response_str[128];

    sprintf(response_str, "{\"current_value\":\"%d.%03d\",\"power_value\":\"%d.%03d\"}",
            current_int, current_frac, power_int, power_frac);
    emMCP_ResponseValue(response_str);
}
