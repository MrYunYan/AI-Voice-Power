#ifndef __PWR_CTL_H__
#define __PWR_CTL_H__

#include "emMCP.h"
#include "ina226.h"

// 定义系统控制状态结构体
typedef struct {
    float target_v;       // 设定的目标电压 (通过按键或串口修改)
    float real_v;         // INA226读取的实际电压
    uint8_t is_output_on; // 输出开关状态 (0:关闭, 1:开启)
    uint8_t is_pps;       // 是否为PPS模式 (0:固定档位, 1:PPS模式)
} SysCtrl_t;

extern SysCtrl_t sys_ctrl;


extern emMCP_tool_t relay;
extern INA226_Device_t my_power_monitor;

void handle_key_voltage_change(uint8_t key_V);

void handle_serial_pps_change(float pps_val);

// 继电器控制回调函数
void emMCP_SetRelayHandler(void *arg);
void emMCP_GetRelayHandler(void *arg);

// 电压控制回调函数
void emMCP_SetVoltageHandler(void *arg);
void emMCP_GetVoltageHandler(void *arg);

// 电流查询回调函数
void emMCP_GetCurrentHandler(void *arg);

// 功率查询回调函数
void emMCP_GetPowerHandler(void *arg);


#endif