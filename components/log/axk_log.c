#include "axk_log.h"
#include "usart.h"

#define AXK_LOG_BUFFER_SIZE 256

AxkLogLevel g_axk_log_level = AXK_LOG_LEVEL_DEBUG;

/*
 * axk_log_printf：格式化日志输出到串口
 * @brief  通过 UART2 发送格式化后的日志字符串
 * @param  format：printf 格式字符串
 * @param  ...：可变参数列表
 * @return 无
 * @note   由日志宏调用，不直接使用；输出到 huart2
 * @author Master_Yang
 * @date   2025-09-29
 */
void axk_log_printf(const char *format, ...)
{
    char buffer[AXK_LOG_BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, AXK_LOG_BUFFER_SIZE, format, args);
    va_end(args);
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, 100);
}

/*
 * axk_log_init：初始化日志串口
 * @brief  初始化 UART1 用于调试日志输出
 * @param  无
 * @return 无
 * @note   调用 MX_USART1_UART_Init 初始化硬件
 * @author Master_Yang
 * @date   2025-09-29
 */
void axk_log_init(void)
{
    MX_USART1_UART_Init();
}
