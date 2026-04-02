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
    // 创建 JSON 对象用于响应
    cJSON *response = cJSON_CreateObject();
    // 读取 GPIOB PIN_5 的电平，判断继电器（LED）状态
    if (HAL_GPIO_ReadPin(OutputKey_GPIO_Port, OutputKey_Pin) == GPIO_PIN_SET) {
        // 如果为高电平，表示 LED/继电器为开
        cJSON_AddBoolToObject(response, "power_state", true);
    } else {
        // 如果为低电平，表示 LED/继电器为关
        cJSON_AddBoolToObject(response, "power_state", false);
    }
    // 将 JSON 对象序列化为字符串
    char *response_str = cJSON_PrintUnformatted(response);
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);

   
    // 释放 JSON 对象占用的内存
    cJSON_Delete(response);
     emMCP_free(response_str); // 释放序列化字符串占用的内存
}

/**
 * @brief  设置电压值的回调函数
 * @param  arg 接收的 JSON 参数对象指针
 * @note   该函数用于设置输出电压值，电压范围 5.0V - 20.0V，精度 0.1V
 *         若接收到 null 值，则查询当前电压（暂不实现查询功能）
 */
void emMCP_SetVoltageHandler(void *arg)
{
    cJSON *param = (cJSON *)arg;
    cJSON *voltage_value = cJSON_GetObjectItem(param, "voltage_value");
    
    if (voltage_value != NULL) {
        if (voltage_value->type == cJSON_NULL) {
            // 查询当前电压 - 暂不实现
            emMCP_ResponseValue("{\"voltage_value\":\"query pending\"}");
        } else {
            // 设置电压值
            float voltage = (float)voltage_value->valuedouble;
            
            // 验证电压范围
            if (voltage >= 5.0f && voltage <= 20.0f) {
                if (axk_ch224_set_pps_vout(voltage) == 0) {
                    // 设置成功
                    char response[64];
                    snprintf(response, sizeof(response), "{\"voltage_value\":%.1f}", voltage);
                    emMCP_ResponseValue(response);
                } else {
                    emMCP_ResponseValue("{\"error\":\"voltage set failed\"}");
                }
            } else {
                emMCP_ResponseValue("{\"error\":\"voltage out of range, 5.0-20.0V\"}");
            }
        }
    }
}

/**
 * @brief  查询电压值的回调函数
 * @param  arg 未使用参数
 * @note   该函数用于查询当前输出电压值
 */
void emMCP_GetVoltageHandler(void *arg)
{
    // 创建 JSON 对象用于响应
    cJSON *response = cJSON_CreateObject();
    
    // 获取总线电压
    float voltage = INA226_GetBusVoltage();


    // 添加电压值到响应
    cJSON_AddNumberToObject(response, "voltage_value", voltage);
    
    // 将 JSON 对象序列化为字符串
    char *response_str = cJSON_PrintUnformatted(response);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
     cJSON_free(response_str);
    // 释放 JSON 对象占用的内存
    cJSON_Delete(response);
}

/**
 * @brief  查询电流值的回调函数
 * @param  arg 未使用参数
 * @note   该函数用于查询当前电流值
 */
void emMCP_GetCurrentHandler(void *arg)
{
    // 创建 JSON 对象用于响应
    cJSON *response = cJSON_CreateObject();
    
    // 获取电流值
    float current = INA226_GetCurrent(&my_power_monitor);
    char formatted[16];
// 单位：A，保留 3 位小数（如 1.234 A）
snprintf(formatted, sizeof(formatted), "%.3f", current);
    // 添加电流值到响应
    cJSON_AddStringToObject(response, "current_value", formatted);
    
    // 将 JSON 对象序列化为字符串
    char *response_str = cJSON_PrintUnformatted(response);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
    cJSON_free(response_str);
    // 释放 JSON 对象占用的内存
   // cJSON_Delete(response);
}

/**
 * @brief  查询功率值的回调函数
 * @param  arg 未使用参数
 * @note   该函数用于查询当前功率值
 */
void emMCP_GetPowerHandler(void *arg)
{
    // 创建 JSON 对象用于响应
    cJSON *response = cJSON_CreateObject();
    
    // 获取功率值
    float power = INA226_GetPower(&my_power_monitor);
    
    // 添加功率值到响应
    cJSON_AddNumberToObject(response, "power_value", power);
    
    // 将 JSON 对象序列化为字符串
    char *response_str = cJSON_PrintUnformatted(response);
    
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
    
    // 释放 JSON 对象占用的内存
    cJSON_Delete(response);
}