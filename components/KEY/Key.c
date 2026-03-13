#include "Key.h"
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "axk_ssd1306.h"//
#include "cmsis_os2.h"
#include "ina226.h"

extern osMessageQueueId_t BtnQueueHandle;
void KEY_NUM(void) {
    uint8_t btn_num = 0;
//  检查按键 1 (切换键)
    if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
        osDelay(10); // 消抖处理
        if (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
            btn_num = 1;
            osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0); // 建议非阻塞发送 (timeout=0)
            while (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port, SwitchKey_Pin) == GPIO_PIN_RESET) {
                osDelay(20); // 等待松开，20ms 检查一次即可，节省 CPU
            }
        }
    }

    
    // 检查按键 2 (增加键)
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
    // 检查按键 3 (减小键)
    // else if (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
    //     osDelay(10);
    //     if (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
    //         btn_num = 3;
    //         osMessageQueuePut(BtnQueueHandle, &btn_num, 0, 0);
    //         while (HAL_GPIO_ReadPin(NextKey_GPIO_Port, NextKey_Pin) == GPIO_PIN_RESET) {
    //             osDelay(20);
    //         }
    //     }
    // }
} 




void KEY_Output(uint8_t num){
// switch (num) {
//         case 0://设置关
//  HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_RESET);
//         break;    
//     case 1://设置开
//  HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_SET);
//         break;
//     case 2://翻转
//  HAL_GPIO_TogglePin(OutputKey_GPIO_Port, OutputKey_Pin);
//         break;

// }

}

void KEY_state(void){

    // if (HAL_GPIO_ReadPin(OutputKey_GPIO_Port,OutputKey_Pin)==GPIO_PIN_SET){
    //     axk_ssd1306_show_utf8_str(92, 6, "开");
    
    // } else {
    //     axk_ssd1306_show_utf8_str(92, 6, "关");
    // }
}







// uint32_t KEY_V_value(void){

// static char buffer[10];
//    if(KEY_NUM()==2){
//     value=value+0.1;
//    //  axk_ssd1306_show_numble(64, 0, 0, 0,value);
//    } else if(KEY_NUM()==3){
//     value=value-0.1;
//    //  axk_ssd1306_show_numble(64, 0, 0, 0,value);
//   }
// snprintf(buffer, sizeof(buffer), "%2d.%1d", int_part, dec_part);
//   axk_ssd1306_show_utf8_str(68, 0, buffer);
//   return value;
// }

