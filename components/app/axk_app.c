#include "axk_app.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "emMCP.h"
#include "axk_ssd1306.h"
#include "axk_ch224.h"
#include "axk_pwr_ctl.h"
#include "axk_key.h"
#include "axk_ina226.h"
#include "usart.h"
#include "uartPort.h"

#define AXK_UART_RXBUFF_MAX 256

static uint8_t s_axk_rx_buffer[AXK_UART_RXBUFF_MAX] = {0};
static emMCP_t s_axk_emmcp_dev;

extern osMessageQueueId_t BtnQueueHandle;


/*
 * axk_app_init_display：初始化 OLED 显示屏
 * @brief  初始化 SSD1306 并显示开机画面
 * @param  无
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_init_display(void)
{
    axk_ssd1306_init();
    axk_ssd1306_set_color_turn(0);
    axk_ssd1306_set_display_turn(0);
    axk_ssd1306_clear_screen();

    axk_ssd1306_show_utf8_str(24, 0, "Dev Init...");
}


/*
 * axk_app_init_power：初始化电源控制外设
 * @brief  初始化 CH224 协议芯片和 INA226 功率监测芯片
 * @param  无
 * @return 无
 * @note   静态函数，仅本文件内可调用；CH224 初始化失败不阻塞
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_init_power(void)
{
    if (axk_ch224_init() == 0) {
        axk_ssd1306_show_utf8_str(24, 2, "CH224 OK");

        if (axk_ch224_set_mode(AXK_CH224_VOUT_PPS) == 0) {
            axk_pwr_handle_serial_pps_change(5.2f);
        } else {
            axk_ch224_set_vout(AXK_CH224_VOUT_5V);
        }
    } else {
        axk_ssd1306_show_utf8_str(24, 2, "CH224 ERROR");
        axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
    }

    if (axk_ina226_init(&g_axk_power_monitor, 0.01f, 3.0f) == 0) {
        axk_ssd1306_show_utf8_str(24, 4, "INA226  OK");
    } else {
        axk_ssd1306_show_utf8_str(24, 4, "INA226 ERROR");
        axk_ssd1306_show_utf8_str(24, 6, "I2C Cfg & HW");
    }
}


/*
 * axk_app_show_main_ui：显示主界面固定元素
 * @brief  显示电压、电流、功率、状态标签
 * @param  无
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_show_main_ui(void)
{
    axk_ssd1306_clear_screen();

    axk_ssd1306_show_utf8_str(0, 0, "电压(V):");
    axk_ssd1306_show_utf8_str(0, 2, "电流(A):");
    axk_ssd1306_show_utf8_str(0, 4, "功率(W):");
    axk_ssd1306_show_utf8_str(0, 6, "当前状态:");
}


/*
 * axk_app_process_key_msg：处理按键消息
 * @brief  根据按键编号执行输出开关、电压加档、电压减档操作
 * @param  key_num：按键编号，1：输出翻转，2：电压加档，3：电压减档
 * @param  key_v：电压档位指针，用于维护当前档位
 * @return 无
 * @note   静态函数，仅本文件内可调用；key_v 在 0~4 范围内循环
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_process_key_msg(uint8_t key_num, int8_t *key_v)
{
    switch (key_num) {
        case 1:
            axk_key_output(2);
            g_axk_sys_ctrl.is_output_on = !g_axk_sys_ctrl.is_output_on;
            break;
        case 2:
            (*key_v)++;
            if ((*key_v) > 4) {
                *key_v = 0;
            }
            axk_pwr_handle_key_voltage(*key_v);
            break;
        case 3:
            (*key_v)--;
            if ((*key_v) < 0) {
                *key_v = 4;
            }
            axk_pwr_handle_key_voltage(*key_v);
            break;
    }
}


/*
 * axk_app_refresh_screen：刷新屏幕显示内容
 * @brief  读取电压、电流、功率并刷新 OLED 显示
 * @param  无
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_refresh_screen(void)
{
    g_axk_sys_ctrl.real_v = axk_ina226_get_bus_voltage();

    if (g_axk_sys_ctrl.is_output_on == 0) {
        axk_ssd1306_show_float(72, 0, AXK_FONT_SIZE_16, 0, g_axk_sys_ctrl.target_v, 1);
    } else {
        axk_ssd1306_show_float(72, 0, AXK_FONT_SIZE_16, 0, g_axk_sys_ctrl.real_v, 1);
    }

    float current = axk_ina226_get_current(&g_axk_power_monitor);
    axk_ssd1306_show_float(72, 2, AXK_FONT_SIZE_16, 0, current, 3);

    float power = axk_ina226_get_power(&g_axk_power_monitor);
    axk_ssd1306_show_float(72, 4, AXK_FONT_SIZE_16, 0, power, 3);
}


/*
 * axk_app_init_emmcp：初始化 emMCP 协议栈及工具注册
 * @brief  启动 UART DMA 空闲接收，初始化 emMCP 并注册输出/电压/参数工具
 * @param  无
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_app_init_emmcp(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)s_axk_rx_buffer, sizeof(s_axk_rx_buffer));
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);

    emMCP_Init(&s_axk_emmcp_dev);
    emMCP_SetAiWakeUp(15);
    emMCP_SetAiVolume(100);
    static emMCP_tool_t output_tool = {
        .name = "设备输出开关",
        .description = "用于设置与查询设备输出状态",
        .inputSchema.properties[0].name = "power_state",
        .inputSchema.properties[0].description = "控制设备输出开关,打开:true,关闭为:false,查询为null",
        .inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_BOOLEAN,
        .setRequestHandler = axk_pwr_set_relay_handler,
        .checkRequestHandler = axk_pwr_get_relay_handler,
    };

    static emMCP_tool_t voltage_tool = {
        .name = "电压值",
        .description = "用来设置与查询电压值",
        .inputSchema.properties[0].name = "voltage_value",
        .inputSchema.properties[0].description =
            "设置电压值,设置为具体的电压数值,范围为5.0伏到20.0伏,设置精度为小数点一位,单位为伏,查询电压值发送null",
        .inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NUMBER,
        .setRequestHandler = axk_pwr_set_voltage_handler,
        .checkRequestHandler = axk_pwr_get_voltage_handler,
    };

    static emMCP_tool_t output_params_tool = {
        .name = "输出电流与功率参数",
        .description = "用来查询电流值与功率值",
        .inputSchema.properties[0].name = "current_value",
        .inputSchema.properties[0].description = "查询电流值发送null,单位:安培",
        .inputSchema.properties[0].type = MCP_SERVER_TOOL_TYPE_NULL,
        .inputSchema.properties[1].name = "power_value",
        .inputSchema.properties[1].description = "查询功率值发送null,单位:瓦",
        .inputSchema.properties[1].type = MCP_SERVER_TOOL_TYPE_NULL,
        .checkRequestHandler = axk_pwr_get_output_params_handler,
    };

    emMCP_AddToolToToolList(&output_tool);
    emMCP_AddToolToToolList(&voltage_tool);
    emMCP_AddToolToToolList(&output_params_tool);

    emMCP_RegistrationTools();
}


/*
 * axk_app_default_task：FreeRTOS 默认任务入口
 * @brief  完成外设初始化，进入主循环处理按键消息并刷新屏幕
 * @param  无
 * @return 无
 * @note   由 CubeMX 生成的 StartDefaultTask 调用；循环周期约 50ms
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_app_default_task(void)
{
    uint8_t key_num = 0;
    int8_t key_v = 0;

    axk_app_init_display();
    axk_app_init_power();

    osDelay(1000);
    axk_app_show_main_ui();

    for (;;) {
        osStatus_t status = osMessageQueueGet(BtnQueueHandle, &key_num, NULL, 0);

        if (status == osOK) {
            axk_app_process_key_msg(key_num, &key_v);
            key_num = 0;
        }

        axk_key_state();
        axk_app_refresh_screen();

        osDelay(50);
    }
}


/*
 * axk_app_key_task：FreeRTOS 按键任务入口
 * @brief  初始化 emMCP 和 UART，循环执行按键扫描与协议处理
 * @param  无
 * @return 无
 * @note   由 CubeMX 生成的 StartKeyTask 调用；任务优先级 osPriorityLow
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_app_key_task(void)
{
    axk_app_init_emmcp();

    for (;;) {
        emMCP_TickHandle(10);
        axk_key_scan();
        osDelay(10);
    }
}


/*
 * axk_app_uart_rx_callback：UART 接收事件回调
 * @brief  处理 UART 空闲中断，重启 DMA 接收并将数据传递给 emMCP 解析
 * @param  huart：UART 句柄指针
 * @param  size：本次接收到的字节数
 * @return 无
 * @note   由 HAL_UARTEx_RxEventCallback 在中断上下文中调用；中断优先级 5；禁止调用耗时函数
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_app_uart_rx_callback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance != USART1) {
        return;
    }

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)s_axk_rx_buffer, sizeof(s_axk_rx_buffer));
    uartPortRecvData((char *)s_axk_rx_buffer, size);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}
