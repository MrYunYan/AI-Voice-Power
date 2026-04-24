/**
 * @file uartPort.c
 * @author Seahi-Mo (seahi-mo@foxmail.com)
 * @brief
 * @version 0.1
 * @date 2025-09-30
 *
 * @copyright Ai-Thinker co.,ltd (c) 2025
 *
 */
#include "uartPort.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* FreeRTOS 与 HAL 头文件 */
#include "cmsis_os.h"
#include "usart.h"

/* Private variables ---------------------------------------------------------*/
/* 注意：uart_data_buf 在 emMCP.c 中定义，这里使用 extern 声明 */
extern char *uart_data_buf;

/* 引用 CubeMX 生成的 FreeRTOS 对象句柄 */
extern osMutexId_t uart1TxMutexHandle;
extern osSemaphoreId_t uart1TxDoneSemHandle;

/* 接收缓冲区与状态（供 uartPortGetRxData / uartPortClearRxData 使用） */
static char s_uart_rx_buf[512];
static volatile int s_uart_rx_ready = 0;

/* uartPort.h 中声明的调试变量 */
volatile uint32_t g_uart_rx_count = 0;
volatile uint32_t g_uart_rx_size = 0;

/* Private function prototypes -----------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

/**
 * @brief 串口发送函数接口
 *
 * @param data 要发送的数据
 * @param len 数据长度
 * @return int 成功返回0，失败返回负值
 */
int uartPortSendData(char *data, int len)
{
    if (data == NULL || len <= 0)
    {
        return -1;
    }

    /* 串行化：防止多任务同时操作 UART1 TX DMA */
    osMutexAcquire(uart1TxMutexHandle, osWaitForever);

    if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, len) != HAL_OK)
    {
        osMutexRelease(uart1TxMutexHandle);
        return -1;
    }

    /* 阻塞当前任务，直到 DMA 完成中断释放信号量 */
    osSemaphoreAcquire(uart1TxDoneSemHandle, osWaitForever);

    osMutexRelease(uart1TxMutexHandle);
    return 0;
}

/**
 * @brief 串口接收函数接口，把这个函数在串口接收中断或接收循环中调用
 *
 * @param data 数据缓冲区
 * @param len 缓冲区长度
 * @return int 成功返回0，失败返回负值
 */
int uartPortRecvData(char *data, int len)
{
    if (data == NULL || len <= 0)
    {
        emMCP_log_error("uartPortRecvData: data is NULL");
        return -1;
    }

    /* 将接收数据复制到本地静态缓冲区，避免调用方释放后数据丢失 */
    int copy_len = (len < (int)sizeof(s_uart_rx_buf) - 1) ? len : (int)sizeof(s_uart_rx_buf) - 1;
    memcpy(s_uart_rx_buf, data, copy_len);
    s_uart_rx_buf[copy_len] = '\0';
    s_uart_rx_ready = 1;

    g_uart_rx_count++;
    g_uart_rx_size = copy_len;

   // uart_data_buf = data;
    emMCP_UpdateUartRecv(true);
    return 0;
}

char *uartPortGetRxData(void)
{
    if (s_uart_rx_ready)
    {
        return (char *)s_uart_rx_buf;
    }
    return NULL;
}

void uartPortClearRxData(void)
{
    s_uart_rx_ready = 0;
    g_uart_rx_size = 0;
}

/**
 * @brief 设置UART数据缓冲区指针
 *
 * @param buf 缓冲区指针
 */
void uartPortSetDataBuf(char *buf)
{
    uart_data_buf = buf;
}

/**
 * @brief 获取UART数据缓冲区指针
 *
 * @return char* 缓冲区指针
 */
char *uartPortGetDataBuf(void)
{
    return uart_data_buf;
}

/**
 * @brief UART DMA 发送完成回调（覆盖 HAL 的 __weak 默认实现）
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        osSemaphoreRelease(uart1TxDoneSemHandle);
    }
}
