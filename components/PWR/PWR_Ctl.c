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
  cJSON *enable = cJSON_GetObjectItem(param, "enable");//获取继电器命令
  if (enable != NULL) {
    if (enable->valueint == 1) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    } else if (enable->valueint == 0) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    emMCP_ResponseValue(emMCP_CTRL_OK); //返回控制成功
  }else {
    emMCP_ResponseValue(emMCP_CTRL_ERROR); //返回控制失败
  }
}
//查询回调函数
void emMCP_GetRelayHandler(void *arg){}
