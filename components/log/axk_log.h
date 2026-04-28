/**
 * @file   axk_log.h
 * @brief  日志模块头文件 — 提供分级日志输出宏和初始化接口
 * @author Master_Yang
 * @date   2025-09-29
 */

#ifndef AXK_LOG_H
#define AXK_LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ANSI 颜色控制码 */
#define AXK_ANSI_COLOR_RESET   "\033[0m"
#define AXK_ANSI_COLOR_GREEN   "\033[32m"
#define AXK_ANSI_COLOR_RED     "\033[31m"
#define AXK_ANSI_COLOR_YELLOW  "\033[33m"
#define AXK_ANSI_COLOR_BLUE    "\033[34m"
#define AXK_ANSI_COLOR_MAGENTA "\033[35m"
#define AXK_ANSI_COLOR_CYAN    "\033[36m"

/* 日志级别枚举 */
typedef enum {
    AXK_LOG_LEVEL_DEBUG,
    AXK_LOG_LEVEL_INFO,
    AXK_LOG_LEVEL_WARN,
    AXK_LOG_LEVEL_ERROR,
    AXK_LOG_LEVEL_FATAL
} AxkLogLevel;

void axk_log_init(void);
void axk_log_printf(const char *format, ...);
extern AxkLogLevel g_axk_log_level;

/* 日志输出宏定义 */
#define AXK_LOG_DEBUG(...)                                                       \
    do {                                                                         \
        if (g_axk_log_level <= AXK_LOG_LEVEL_DEBUG)                              \
        {                                                                        \
            axk_log_printf(AXK_ANSI_COLOR_BLUE "[DEBUG] %s:%d: ", __func__,      \
                           __LINE__);                                            \
            axk_log_printf(__VA_ARGS__);                                         \
            axk_log_printf(AXK_ANSI_COLOR_RESET "\r\n");                         \
        }                                                                        \
    } while (0)

#define AXK_LOG_INFO(...)                                                        \
    do {                                                                         \
        if (g_axk_log_level <= AXK_LOG_LEVEL_INFO)                               \
        {                                                                        \
            axk_log_printf(AXK_ANSI_COLOR_GREEN "[INFO] %s:%d: ", __func__,      \
                           __LINE__);                                            \
            axk_log_printf(__VA_ARGS__);                                         \
            axk_log_printf(AXK_ANSI_COLOR_RESET "\r\n");                         \
        }                                                                        \
    } while (0)

#define AXK_LOG_WARN(...)                                                        \
    do {                                                                         \
        if (g_axk_log_level <= AXK_LOG_LEVEL_WARN)                               \
        {                                                                        \
            axk_log_printf(AXK_ANSI_COLOR_YELLOW "[WARN] %s:%d: ", __func__,     \
                           __LINE__);                                            \
            axk_log_printf(__VA_ARGS__);                                         \
            axk_log_printf(AXK_ANSI_COLOR_RESET "\r\n");                         \
        }                                                                        \
    } while (0)

#define AXK_LOG_ERROR(...)                                                       \
    do {                                                                         \
        if (g_axk_log_level <= AXK_LOG_LEVEL_ERROR)                              \
        {                                                                        \
            axk_log_printf(AXK_ANSI_COLOR_RED "[ERROR] %s:%d: ", __func__,       \
                           __LINE__);                                            \
            axk_log_printf(__VA_ARGS__);                                         \
            axk_log_printf(AXK_ANSI_COLOR_RESET "\r\n");                         \
        }                                                                        \
    } while (0)

#define AXK_LOG_FATAL(...)                                                       \
    do {                                                                         \
        if (g_axk_log_level <= AXK_LOG_LEVEL_FATAL)                              \
        {                                                                        \
            axk_log_printf(AXK_ANSI_COLOR_MAGENTA "[FATAL] %s:%d: ", __func__,   \
                           __LINE__);                                            \
            axk_log_printf(__VA_ARGS__);                                         \
            axk_log_printf(AXK_ANSI_COLOR_RESET "\r\n");                         \
        }                                                                        \
    } while (0)

#endif /* AXK_LOG_H */
