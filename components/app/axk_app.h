#ifndef AXK_APP_H
#define AXK_APP_H

#include "main.h"
#include "cmsis_os.h"

void axk_app_default_task(void);
void axk_app_key_task(void);
void axk_app_uart_rx_callback(UART_HandleTypeDef *huart, uint16_t size);

#endif /* AXK_APP_H */
