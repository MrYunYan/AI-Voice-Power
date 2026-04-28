/**
 * @file   axk_bsp_i2c.h
 * @brief  I2C 软件模拟底层驱动头文件 — GPIO 位操作宏和接口声明
 * @author Master_Yang
 * @date   2026-04-27
 */

#ifndef AXK_BSP_I2C_H
#define AXK_BSP_I2C_H

#include "stm32f10x_delay.h"
#include "axk_log.h"
#include <stdint.h>

#define AXK_DELAY_US(x) HAL_Delay_us(x)
#define AXK_DELAY_MS(x) HAL_Delay(x)

#define AXK_BSP_I2C_RCC   RCC_APB2Periph_GPIOB
#define AXK_BSP_I2C_PORT  GPIOB
#define AXK_BSP_I2C_SDA_PIN  GPIO_PIN_7
#define AXK_BSP_I2C_SCL_PIN  GPIO_PIN_6
#define AXK_BSP_I2C_WRITE 0x00
#define AXK_BSP_I2C_READ  0x01

#define AXK_BSP_I2C_SDA_IN()                                                   \
    do {                                                                       \
        GPIO_InitTypeDef GPIO_InitStruct = {0};                                \
        GPIO_InitStruct.Pin = AXK_BSP_I2C_SDA_PIN;                                 \
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                \
        GPIO_InitStruct.Pull = GPIO_NOPULL;                                    \
        HAL_GPIO_Init(AXK_BSP_I2C_PORT, &GPIO_InitStruct);                     \
    } while (0)

#define AXK_BSP_I2C_SDA_OUT()                                                  \
    do {                                                                       \
        GPIO_InitTypeDef GPIO_InitStruct = {0};                                \
        GPIO_InitStruct.Pin = AXK_BSP_I2C_SDA_PIN;                                 \
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                            \
        GPIO_InitStruct.Pull = GPIO_NOPULL;                                    \
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                          \
        HAL_GPIO_Init(AXK_BSP_I2C_PORT, &GPIO_InitStruct);                     \
    } while (0)

#define AXK_BSP_I2C_SDA_GET() HAL_GPIO_ReadPin(AXK_BSP_I2C_PORT, AXK_BSP_I2C_SDA_PIN)
#define AXK_BSP_I2C_SDA(x)    HAL_GPIO_WritePin(AXK_BSP_I2C_PORT, AXK_BSP_I2C_SDA_PIN, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define AXK_BSP_I2C_SCL(x)    HAL_GPIO_WritePin(AXK_BSP_I2C_PORT, AXK_BSP_I2C_SCL_PIN, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))

void     axk_bsp_i2c_init(void);
void     axk_bsp_i2c_start(void);
void     axk_bsp_i2c_stop(void);
void     axk_bsp_i2c_send_ack(uint8_t ack);
uint8_t  axk_bsp_i2c_wait_ack(void);
void     axk_bsp_i2c_send_byte(uint8_t dat);
uint8_t  axk_bsp_i2c_read_byte(void);

#endif /* AXK_BSP_I2C_H */
