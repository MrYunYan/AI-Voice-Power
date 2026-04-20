/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "axk_ssd1306.h"
#include "emMCP.h"
#include "uartPort.h"
#include "axk_ch224.h"
#include "PWR_Ctl.h" 
#include "Key.h"
#include "ina226.h"
#include <stdint.h>
#include <sys/_types.h>
#include <math.h>
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// 不定长数据接收完成回调函数

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for BtnQueue */
osMessageQueueId_t BtnQueueHandle;
const osMessageQueueAttr_t BtnQueue_attributes = {
  .name = "BtnQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#define UART_RXBUFF_MAX 256
//static emMCP_t emMCP;
static uint8_t rxBuffer[UART_RXBUFF_MAX] = {0};
static emMCP_t emMCP_dev;
INA226_Device_t my_power_monitor;  // 全局 INA226 设备管理器，供回调函数使用

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartKeyTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of BtnQueue */
  BtnQueueHandle = osMessageQueueNew (16, sizeof(uint8_t), &BtnQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

//HAL_UART_Transmit(&huart1, "Hello, UART1!\r\n", 13, 100);
//HAL_UART_Transmit(&huart2, "Hello, UART2!\r\n", 13, 100);


  axk_ssd1306_init();
  axk_ssd1306_set_color_turn(0);
  axk_ssd1306_set_display_turn(0);
  axk_ssd1306_clear_screen();

axk_ssd1306_show_utf8_str(24, 0, "Dev Init...");
//初始化CH224
 if (axk_ch224_init() == 0) { 
   axk_ssd1306_show_utf8_str(24, 2, "CH224 OK");


  if (axk_ch224_set_mode(AXK_CH224_VOUT_PPS) == 0) {
    handle_serial_pps_change(5.2f);
  }else {
    axk_ch224_set_vout(AXK_CH224_VOUT_5V);
  }
  }else {
   axk_ssd1306_show_utf8_str(24, 2, "CH224 ERROR");
     axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
  // while (1) {//死循环停在这里
  // }
  }

/* 
* 初始化 INA226
* 假设你的硬件电路上使用的是 10 毫欧 (0.01 欧姆) 的采样电阻
* 并且你的可调电源预计最大电流为 2.0 A
*/
 if (INA226_Init(&my_power_monitor, 0.01f, 3.0f) == 0) {               
     axk_ssd1306_show_utf8_str(24, 4, "INA226  OK");
 } else {
     axk_ssd1306_show_utf8_str(24, 4, "INA226 ERROR");
     axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
    //  while (1) {//死循环停在这里
    //   }
 }
 


osDelay(1000);
axk_ssd1306_clear_screen();//初始化完成清屏
   

  axk_ssd1306_show_utf8_str(0, 0,"电压(V):");
  axk_ssd1306_show_utf8_str(0, 2, "电流(A):");


  axk_ssd1306_show_utf8_str(0, 4, "功率(W):");
 
  axk_ssd1306_show_utf8_str(0, 6, "当前状态:");
  //axk_ssd1306_show_utf8_str(92, 6, "关");







//char buffer[10];
 uint8_t key_num=0;
 int8_t key_V=0;
 for(;;){
    // 尝试从队列中获取按键消息，并把结果保存在 status 变量里
    // 这里的 0 表示如果没有消息就立刻继续往下走，不卡在这里死等
    osStatus_t status = osMessageQueueGet(BtnQueueHandle, &key_num, NULL, 0);
    
    // 只有当状态是 osOK（代表成功拿到了新的按键消息）时，才去处理按键
    if (status == osOK) {
        
        switch (key_num) {
            case 1:
               KEY_Output(2); // 设置输出反转
               sys_ctrl.is_output_on = !sys_ctrl.is_output_on; // 更新状态
                break;
            case 2:
                key_V++;       // 电压档位加 1
                if(key_V > 4){
                    key_V = 0;   // 如果超过最大档位，回到 0 档
                }
                  handle_key_voltage_change(key_V); // 根据当前按键档位设置电压输出
                break;
            case 3:
                key_V--;       // 电压档位减 1
                if(key_V < 0){
                    key_V = 4;   // 如果小于最小档位，循环到最大档
                    
                }
                 handle_key_voltage_change(key_V); // 根据当前按键档位设置电压输出
                break;
        }
        
        // 处理完一次按键后，立刻把按键值清零，防止下一圈循环重复触发
        key_num = 0; 
       
    }

    KEY_state();   // 读取输出引脚状态并显示
    // 下面的代码不受按键影响，每次循环都会正常刷新屏幕和电压



     //
      // 读取并显示电压
   sys_ctrl.real_v = INA226_GetBusVoltage();
    // 2. 根据开关状态决定显示逻辑
        if (sys_ctrl.is_output_on == 0) {
            // 【输出关闭状态】：显示我们设定的目标电压
            // 使用 sprintf 将浮点数格式化为字符串，保留1位或2位小数
          
             axk_ssd1306_show_float(72, 0, FONT_SIEZE_16, 0, sys_ctrl.target_v, 1); //显示电压
            // 可选体验优化：关闭状态下，可以加上闪烁或者反色效果，提醒用户当前没有电输出
            
        } else {
            // 【输出开启状态】：显示 INA226 测量到的真实电压
        
             axk_ssd1306_show_float(72, 0, FONT_SIEZE_16, 0, sys_ctrl.real_v, 1); // 显示电压
      
        }
    // 读取并显示电流
    float current = INA226_GetCurrent(&my_power_monitor);
   axk_ssd1306_show_float(72, 2, FONT_SIEZE_16, 0, current, 3); // 显示电流，保留3位小数
    // 读取并显示功率
    float power = INA226_GetPower(&my_power_monitor);
   axk_ssd1306_show_float(72, 4, FONT_SIEZE_16, 0, power, 3); // 显示功率，保留3位小数
    // 延时 50 毫秒
    osDelay(50);
 }

  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)rxBuffer, sizeof(rxBuffer));
__HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);//关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）

//九
//HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxBuffer, sizeof(rxBuffer));
//__HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);//关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）

  
 
 emMCP_Init(&emMCP_dev);

 static emMCP_tool_t Output;//创建工具
  Output.name = "设备输出开关";//工具名称，保持唯一性
  Output.description = "用于设置与查询设备输出状态";//工具的功能描述
  Output.inputSchema.properties[0].name = "power_state";//属性指令，AI 通过这个指令发送命令
  Output.inputSchema.properties[0].description = "控制设备输出开关,打开:true,关闭为:false,查询为null";  //指令描述，AI 通过这个描述理解指令
  Output.inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_BOOLEAN;//指令类型，AI 通过这个类型发送相对应的数据
  Output.setRequestHandler = emMCP_SetRelayHandler;//设置控制回调
  Output.checkRequestHandler = emMCP_GetRelayHandler;//设置查询回调
  emMCP_AddToolToToolList(&Output);   // 添加工具到工具列表


  static emMCP_tool_t voltage;//创建工具
  voltage.name = "电压值";//工具名称，保持唯一性
  voltage.description = "用来设置与查询电压值";//工具的功能描述
  voltage.inputSchema.properties[0].name = "voltage_value";//属性指令，AI 通过这个指令发送命令
  voltage.inputSchema.properties[0].description =
   "设置电压值,设置为具体的电压数值,范围为5.0伏到20.0伏,设置精度为小数点一位,单位为伏,查询电压值发送null";  //指令描述，AI 通过这个描述理解指令
  voltage.inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NUMBER;//指令类型，AI 通过这个类型发送相对应的数据
  voltage.setRequestHandler = emMCP_SetVoltageHandler;//设置控制回调
  voltage.checkRequestHandler = emMCP_GetVoltageHandler;//设置查询回调
  emMCP_AddToolToToolList(&voltage);   // 添加工具到工具列表

  static emMCP_tool_t output_params;//创建工具
  output_params.name = "设备输出参数";//工具名称，保持唯一性
  output_params.description = "用来查询电流值与功率值";//工具的功能描述
  output_params.inputSchema.properties[0].name = "current_value";//属性指令，AI 通过这个指令发送命令
  output_params.inputSchema.properties[0].description = "查询电流值发送null,单位:安培";  //指令描述，AI 通过这个描述理解指令
  output_params.inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NUMBER;//指令类型，AI 通过这个类型发送相对应的数据

  output_params.inputSchema.properties[1].name = "power_value";//属性指令，AI 通过这个指令发送命令
  output_params.inputSchema.properties[1].description = "查询功率值发送null,单位:瓦";  //指令描述，AI 通过这个描述理解指令
  output_params.inputSchema.properties[1].type = MCP_SERVER_TOOL_TYPE_NUMBER;//指令类型，AI 通过这个类型发送相对应的数据

  output_params.checkRequestHandler = emMCP_GetOutputParamsHandler;//查询回调
  emMCP_AddToolToToolList(&output_params);   // 添加工具到工具列表



  // static emMCP_tool_t current;//创建工具
  // current.name = "电流值";//工具名称，保持唯一性
  // current.description = "用来查询电流值";//工具的功能描述
  // current.inputSchema.properties[0].name = "current_value";//属性指令，AI 通过这个指令发送命令
  // current.inputSchema.properties[0].description = "查询电流值发送null,单位:安培";  //指令描述，AI 通过这个描述理解指令
  // current.inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NUMBER;//指令类型，AI 通过这个类型发送相对应的数据
  // current.checkRequestHandler = emMCP_GetCurrentHandler;//设置查询回调
  // emMCP_AddToolToToolList(&current);   // 添加工具到工具列表

  // static emMCP_tool_t power;//创建工具
  // power.name = "功率值";//工具名称，保持唯一性
  // power.description = "用来查询功率值";//工具的功能描述
  // power.inputSchema.properties[0].name = "power_value";//属性指令，AI 通过这个指令发送命令
  // power.inputSchema.properties[0].description = "查询功率值发送null,单位:瓦";  //指令描述，AI 通过这个描述理解指令
  // power.inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NUMBER;//指令类型，AI 通过这个类型发送相对应的数据
  // power.checkRequestHandler = emMCP_GetPowerHandler;//设置查询回调
  // emMCP_AddToolToToolList(&power);   // 添加工具到工具列表


  emMCP_RegistrationTools(); // 注册工具到小安AI



  for(;;)
  {
  emMCP_TickHandle(10);
    KEY_NUM();  
    osDelay(10);
  }
  /* USER CODE END StartKeyTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {

    if (huart->Instance == USART1)
    {

       HAL_UARTEx_ReceiveToIdle_DMA(&huart1,  (uint8_t *)rxBuffer,  sizeof(rxBuffer));
  //调用emMCP接收函数
    uartPortRecvData((char *)rxBuffer, Size);
   
        // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }


//九
  //     if (huart->Instance == USART2){     
  // //调用emMCP接收函数
  // uartPortRecvData((char *)rxBuffer, Size);
  //   HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxBuffer, sizeof(rxBuffer));
  //   __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
  //   }
}
/* USER CODE END Application */

