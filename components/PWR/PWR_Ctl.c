#include "PWR_Ctl.h"

#include "emMCP.h"
#include "cJSON.h"
#include "emMCPLOG.h"
#include "gpio.h"
#include "main.h"
#include "axk_ch224.h"
#include "uartPort.h"
#include <stdio.h>
#include "stdio.h" // 用于 sprintf


// 声明一个全局状态实例
SysCtrl_t sys_ctrl = {
    .target_v = 5.0f,     // 默认上电目标为5V
    .real_v = 0.0f,
    .is_output_on = 0,
    .is_pps = 0
};

// --- 按键任务中调用的处理函数 ---
void handle_key_voltage_change(uint8_t key_V) {
    sys_ctrl.is_pps = 0; // 按键切换的是固定档位，退出PPS模式
    
    switch (key_V) {
        case 0:
            axk_ch224_set_vout(AXK_CH224_VOUT_5V);
            sys_ctrl.target_v = 5.0f; // 更新目标电压
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

// --- 串口接收任务中调用的处理函数 ---
void handle_serial_pps_change(float pps_val) {
    // 假设上位机传来了 15.2
    if (axk_ch224_set_pps_vout(pps_val) == 0) { // 如果设置成功
        sys_ctrl.is_pps = 1;         // 进入PPS模式
        sys_ctrl.target_v = pps_val; // 更新目标电压
    }
}


//控制回调函数
void emMCP_SetRelayHandler(void *arg)
{
    // 接收到的数据
  cJSON *param = (cJSON *)arg;
  // 控制继电器
  cJSON *led_state = cJSON_GetObjectItem(param, "power_state");//获取继电器命令
  if (led_state != NULL) {
    if (led_state->valueint == true) {
      HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_SET);
      sys_ctrl.is_output_on = 1; // 更新状态
          emMCP_ResponseValue("{\"power_state\":true}"); //返回控制成功
    } else if (led_state->valueint == false) {
      sys_ctrl.is_output_on = 0; // 更新状态
      HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_RESET);
          emMCP_ResponseValue("{\"power_state\":false}"); //返回控制关闭
    }
  }
}
//查询回调函数
/**
 * @brief  查询继电器（LED）当前状态的回调函数
 * @param  arg 未使用参数（保留接口一致性）
 * @note   该函数会读取 GPIOB 的 PIN_5 引脚电平，判断继电器（LED）当前开关状态，
 *         并将状态以 JSON 格式通过 emMCP_ResponseValue 返回。
 *         主要流程如下：
 *         1. 创建一个 cJSON 对象用于存储响应数据。
 *         2. 读取 GPIOB PIN_5 的电平，判断继电器（LED）是开还是关。
 *         3. 将状态（true/false）添加到 JSON 对象中，键名为 "power_state"。
 *         4. 将 JSON 对象序列化为无格式字符串。
 *         5. 通过 emMCP_ResponseValue 返回 JSON 字符串。
 *         6. 释放 JSON 对象占用的内存。
 */
void emMCP_GetRelayHandler(void *arg)
{
    char response_str[64];
    
    // 读取 GPIOB PIN_5 的电平，判断继电器（LED）状态
    if (HAL_GPIO_ReadPin(OutputKey_GPIO_Port, OutputKey_Pin) == GPIO_PIN_SET) {
        // 如果为高电平，表示 LED/继电器为开
        sprintf(response_str, "{\"power_state\":true}");
    } else {
        // 如果为低电平，表示 LED/继电器为关
        sprintf(response_str, "{\"power_state\":false}");
    }
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);

}

/**
 * @brief 设置电压值的回调函数
 * @param arg 包含电压参数的 JSON 对象指针
 * @note  支持设置5.0V到20.0V的电压，精度为0.1V
 *        函数会将设备设置为PPS模式并更新目标电压
 *        使用手动整数+小数分离方式避免sprintf %f不支持的问题
 */
void emMCP_SetVoltageHandler(void *arg)
{
    // 接收到的数据
    cJSON *param = (cJSON *)arg;
    
    // 获取电压参数
    cJSON *voltage_value = cJSON_GetObjectItem(param, "voltage_value");
    
    if (voltage_value != NULL && voltage_value->valuedouble != 0) {
        float target_voltage = (float)voltage_value->valuedouble;
        
        // 检查电压值是否在有效范围内 (5.0V - 20.0V)
        if (target_voltage >= 5.0f && target_voltage <= 20.0f) {
            // 调用PPS设置函数
            if (axk_ch224_set_pps_vout(target_voltage) == 0) {
                // 设置成功，更新系统状态
                sys_ctrl.is_pps = 1;           // 进入PPS模式
                sys_ctrl.target_v = target_voltage; // 更新目标电压
                
                // 手动拆分：整数部分和小数部分（1位小数）
                int voltage_int = (int)target_voltage;
                int voltage_frac = (int)((target_voltage - voltage_int) * 10);
                if (voltage_frac < 0) {
                    voltage_frac = -voltage_frac;
                }
                
                // 返回成功响应
                char response_str[64];
                sprintf(response_str, "{\"voltage_value\":%d.%01d}", voltage_int, voltage_frac);
                emMCP_ResponseValue(response_str);
            } else {
                // 设置失败
                emMCP_ResponseValue("{\"error\":\"Failed to set voltage\"}");
            }
        } else {
            // 电压值超出范围
            emMCP_ResponseValue("{\"error\":\"Voltage out of range (5.0V - 20.0V)\"}");
        }
    } else {
        // 参数无效
        emMCP_ResponseValue("{\"error\":\"Invalid voltage parameter\"}");
    }
}

/**
 * @brief 查询电压值的回调函数
 * @param arg 未使用参数（保留接口一致性）
 * @note  根据输出开关状态返回不同的电压值：
 *        - 输出关闭时：返回设定的目标电压
 *        - 输出开启时：返回INA226测量的实际电压
 *        使用手动整数+小数分离方式避免sprintf %f不支持的问题
 */
void emMCP_GetVoltageHandler(void *arg)
{
    char response_str[64];
    float voltage_to_report;
    
    // 根据输出开关状态决定返回哪个电压值
    if (sys_ctrl.is_output_on == 0) {
        // 输出关闭时，返回目标电压
        voltage_to_report = sys_ctrl.target_v;
    } else {
        // 输出开启时，返回实际电压
        voltage_to_report = sys_ctrl.real_v;
    }
    
    // 手动拆分：整数部分和小数部分（1位小数）
    int voltage_int = (int)voltage_to_report;
    int voltage_frac = (int)((voltage_to_report - voltage_int) * 10);
    if (voltage_frac < 0) {
        voltage_frac = -voltage_frac;
    }
    
    // 格式化电压值，精度保留1位小数
    sprintf(response_str, "{\"voltage_value\":%d.%01d}", voltage_int, voltage_frac);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
}

/**
 * @brief 查询电流值的回调函数
 * @param arg 未使用参数（保留接口一致性）
 * @note  通过INA226芯片读取当前电流值
 *        使用手动整数+小数分离方式避免sprintf %f不支持的问题
 */
void emMCP_GetCurrentHandler(void *arg)
{
    char response_str[64];
    
    // 通过INA226读取电流值 (单位: 安培)
    float current = INA226_GetCurrent(&my_power_monitor);
    
    // 手动拆分：整数部分和小数部分（3位小数）
    int current_int = (int)current;
    int current_frac = (int)((current - current_int) * 1000);
    if (current_frac < 0) {
        current_frac = -current_frac;
    }
    
    // 格式化电流值，精度保留3位小数
    sprintf(response_str, "{\"current_value\":%d.%03d}", current_int, current_frac);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
}

/**
 * @brief 查询功率值的回调函数
 * @param arg 未使用参数（保留接口一致性）
 * @note  通过INA226芯片读取当前功率值
 *        使用手动整数+小数分离方式避免sprintf %f不支持的问题
 */
void emMCP_GetPowerHandler(void *arg)
{
    char response_str[64];
    
    // 通过INA226读取功率值 (单位: 瓦)
    float power = INA226_GetPower(&my_power_monitor);
    
    // 手动拆分：整数部分和小数部分（3位小数）
    int power_int = (int)power;
    int power_frac = (int)((power - power_int) * 1000);
    if (power_frac < 0) {
        power_frac = -power_frac;
    }
    
    // 格式化功率值，精度保留3位小数
    sprintf(response_str, "{\"power_value\":%d.%03d}", power_int, power_frac);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
}
