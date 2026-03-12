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
#include "cmsis_os2.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "axk_ssd1306.h"
#include "emMCP.h"
#include "axk_ch224.h"
#include "Key.h"
#include "ina226.h"
#include <stdint.h>
#include <sys/_types.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for BtnQueue */
osMessageQueueId_t BtnQueueHandle;
const osMessageQueueAttr_t BtnQueue_attributes = {
  .name = "BtnQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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

  axk_ssd1306_init();
  axk_ssd1306_set_color_turn(0);
  axk_ssd1306_set_display_turn(0);
  axk_ssd1306_clear_screen();

axk_ssd1306_show_utf8_str(24, 0, "Dev Init...");
//初始化CH224
 if (axk_ch224_init() == 0) { 
   axk_ssd1306_show_utf8_str(24, 2, "CH224 OK");

  if (axk_ch224_set_mode(AXK_CH224_VOUT_PPS) == 0) {
    axk_ch224_set_pps_vout(9.0);
  }else {
    axk_ch224_set_vout(AXK_CH224_VOUT_9V);
  }
  }else {
   axk_ssd1306_show_utf8_str(24, 2, "CH224 ERROE");
     axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
  while (1) {//死循环停在这里
  }
  }


/* 实例化一个 INA226 设备管理器 */
INA226_Device_t my_power_monitor;
/* 
* 初始化 INA226
* 假设你的硬件电路上使用的是 10 毫欧 (0.01 欧姆) 的采样电阻
* 并且你的可调电源预计最大电流为 2.0 A
*/
 if (INA226_Init(&my_power_monitor, 0.01f, 2.0f) == 0) {               
     axk_ssd1306_show_utf8_str(24, 4, "INA226  OK");
 } else {
     axk_ssd1306_show_utf8_str(24, 4, "INA226 ERROE");
     axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
    //  while (1) {//死循环停在这里
    //   }
 }

osDelay(2000);
axk_ssd1306_clear_screen();//初始化完成清屏


  axk_ssd1306_show_utf8_str(0, 0,"电压(V):");
  axk_ssd1306_show_utf8_str(0, 2, "电流(mA):");


  axk_ssd1306_show_utf8_str(0, 4, "功率(mW):");
 
  axk_ssd1306_show_utf8_str(0, 6, "当前状态:");
  axk_ssd1306_show_utf8_str(92, 6, "关");

char buffer[10];
 uint8_t key_num=0;
 int8_t key_V=1;
 
 for(;;){
    // 尝试从队列中获取按键消息，并把结果保存在 status 变量里
    // 这里的 0 表示如果没有消息就立刻继续往下走，不卡在这里死等
    osStatus_t status = osMessageQueueGet(BtnQueueHandle, &key_num, NULL, 0);
    
    // 只有当状态是 osOK（代表成功拿到了新的按键消息）时，才去处理按键
    if (status == osOK) {
        switch (key_num) {
            case 1:
                KEY_Output(2); // 设置输出反转
                KEY_state();   // 读取输出引脚状态并显示
                break;
            case 2:
                key_V++;       // 电压档位加 1
                if(key_V > 4){
                    key_V = 0;   // 如果超过最大档位，回到 0 档
                }
                break;
            case 3:
                key_V--;       // 电压档位减 1
                if(key_V < 0){
                    key_V = 4;   // 如果小于最小档位，循环到最大档
                }
                break;
        }
        
        // 处理完一次按键后，立刻把按键值清零，防止下一圈循环重复触发
        key_num = 0; 
    }

    // 下面的代码不受按键影响，每次循环都会正常刷新屏幕和电压
    switch (key_V) {
        case 0:
            axk_ch224_set_vout(AXK_CH224_VOUT_5V);         // 设置输出 5V
            axk_ssd1306_show_utf8_str(88, 0, " 5");        // 屏幕显示 5
            break;
        case 1:
            axk_ch224_set_vout(AXK_CH224_VOUT_9V);         // 设置输出 9V
            axk_ssd1306_show_utf8_str(88, 0, " 9");        // 屏幕显示 9
            break;
        case 2:
            axk_ch224_set_vout(AXK_CH224_VOUT_12V);        // 设置输出 12V
            axk_ssd1306_show_utf8_str(88, 0, "12");        // 屏幕显示 12
            break;
        case 3:
            axk_ch224_set_vout(AXK_CH224_VOUT_15V);        // 设置输出 15V
            axk_ssd1306_show_utf8_str(88, 0, "15");        // 屏幕显示 15
            break;
        case 4:
            axk_ch224_set_vout(AXK_CH224_VOUT_20V);        // 设置输出 20V
            axk_ssd1306_show_utf8_str(88, 0, "20");        // 屏幕显示 20
            break;
    }
       
    // 读取并显示电流
    float current = INA226_GetCurrent(&my_power_monitor);
    int32_t current_mA = (int32_t)round(current * 1000.0f);  // 乘以 900 转换为毫安
    snprintf(buffer, sizeof(buffer), "%4ld", (long)current_mA);
    axk_ssd1306_show_utf8_str(72, 2, buffer);

    // 读取并显示功率
    float power = INA226_GetPower(&my_power_monitor);
    int32_t current_w = (int32_t)round(power * 1000.0f);     // 转换为毫瓦
    snprintf(buffer, sizeof(buffer), "%4ld", (long)current_w);
    axk_ssd1306_show_utf8_str(72, 4, buffer);

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
  for(;;)
  {

    KEY_NUM();  
    osDelay(10);
  }
  /* USER CODE END StartKeyTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

