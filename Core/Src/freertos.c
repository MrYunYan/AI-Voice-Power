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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

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
  axk_ch224_set_vout(AXK_CH224_VOUT_5V);
//   axk_ch224_set_mode(AXK_CH224_VOUT_PPS);
//  axk_ch224_set_pps_vout(5.0);
  axk_ssd1306_show_utf8_str(24, 2, "CH224 OK");
  }else {
   axk_ssd1306_show_utf8_str(24, 2, "CH224 ERROE");
     axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
   while (1) {}
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
     while (1) {}
 }

osDelay(2000);
axk_ssd1306_clear_screen();//初始化完成清屏


  axk_ssd1306_show_utf8_str(0, 0,"电压(V):");
  KEY_V_value();
  axk_ssd1306_show_utf8_str(0, 2, "电流(A):");
  axk_ssd1306_show_utf8_str(0, 4, "功率(W):");
  axk_ssd1306_show_utf8_str(0, 6, "状态:");
  axk_ssd1306_show_utf8_str(48, 6, "关");




  for(;;){
    if(KEY_NUM()==1){
     KEY_Output(2);
     KEY_state();
    }
       KEY_V_value();

       

        
        // osDelay(500);

  }

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

