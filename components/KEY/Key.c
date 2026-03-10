#include "Key.h"
#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "axk_ssd1306.h"//
#include "ina226.h"

uint8_t KEY_NUM(void){
if ((HAL_GPIO_ReadPin(SwitchKey_GPIO_Port,  SwitchKey_Pin))==0) {
osDelay(20);
    if ((HAL_GPIO_ReadPin(SwitchKey_GPIO_Port,  SwitchKey_Pin))==0) {
        while (HAL_GPIO_ReadPin(SwitchKey_GPIO_Port,  SwitchKey_Pin)==0)
        {
            osDelay(1);
        }
   return 1;//输开关
    }
}
else if ((HAL_GPIO_ReadPin(UpKey_GPIO_Port,  UpKey_Pin))==0) {  
    osDelay(20);
    if ((HAL_GPIO_ReadPin(UpKey_GPIO_Port,  UpKey_Pin))==0) { 
        while (HAL_GPIO_ReadPin(UpKey_GPIO_Port,  UpKey_Pin)==0)
          {
            osDelay(1);
        }
    return 2;//上调节
    }
}
else if ((HAL_GPIO_ReadPin(NextKey_GPIO_Port,  NextKey_Pin))==0) {  
osDelay(20);
    if ((HAL_GPIO_ReadPin(NextKey_GPIO_Port,  NextKey_Pin))==0) {
        while (HAL_GPIO_ReadPin(NextKey_GPIO_Port,  NextKey_Pin)==0)
          {
            osDelay(1);
        }
    return 3;//下调节
    }
}
return 0;//无按键按下
} 




void KEY_Output(uint8_t num){
switch (num) {
        case 0://设置关
 HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_RESET);
        break;    
    case 1://设置开
 HAL_GPIO_WritePin(OutputKey_GPIO_Port, OutputKey_Pin, GPIO_PIN_SET);
        break;
    case 2://翻转
 HAL_GPIO_TogglePin(OutputKey_GPIO_Port, OutputKey_Pin);
        break;

}

}

void KEY_state(void){

    if (HAL_GPIO_ReadPin(OutputKey_GPIO_Port,OutputKey_Pin)==GPIO_PIN_SET){
        axk_ssd1306_show_utf8_str(48, 6, "开");
    
    } else {
        axk_ssd1306_show_utf8_str(48, 6, "关");
    }
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

