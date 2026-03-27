#include "PWR_Ctl.h"

#include "emMCP.h"
#include "cJSON.h"
#include "emMCPLOG.h"
#include "gpio.h"
//控制回调函数
void emMCP_SetRelayHandler(void *arg)
{
    // 接收到的数据
  cJSON *param = (cJSON *)arg;
  // 控制继电器
  cJSON *led_state = cJSON_GetObjectItem(param, "led_state");//获取继电器命令
  if (led_state != NULL) {
    if (led_state->valueint == true) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
          emMCP_ResponseValue("{\"led_state\":true}"); //返回控制成功
    } else if (led_state->valueint == false) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
          emMCP_ResponseValue("{\"led_state\":false}"); //返回控制关闭
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
 *         3. 将状态（true/false）添加到 JSON 对象中，键名为 "led_state"。
 *         4. 将 JSON 对象序列化为无格式字符串。
 *         5. 通过 emMCP_ResponseValue 返回 JSON 字符串。
 *         6. 释放 JSON 对象占用的内存。
 */
void emMCP_GetRelayHandler(void *arg)
{
    // 创建 JSON 对象用于响应
    cJSON *response = cJSON_CreateObject();
    // 读取 GPIOB PIN_5 的电平，判断继电器（LED）状态
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET) {
        // 如果为高电平，表示 LED/继电器为开
        cJSON_AddBoolToObject(response, "led_state", true);
    } else {
        // 如果为低电平，表示 LED/继电器为关
        cJSON_AddBoolToObject(response, "led_state", false);
    }
    // 将 JSON 对象序列化为字符串
    char *response_str = cJSON_PrintUnformatted(response);
    // 返回 JSON 字符串作为查询结果
    emMCP_ResponseValue(response_str);
    // 释放 JSON 对象占用的内存
    cJSON_Delete(response);
}