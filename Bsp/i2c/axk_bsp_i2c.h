#ifndef AXK_BSP_I2C_H
#define AXK_BSP_I2C_H

#include "stm32f10x_delay.h"
#include "log.h"
#include <stdint.h>

#define delay_us(x) HAL_Delay_us(x)
#define delay_ms(x) HAL_Delay(x)

#define BSP_I2C_RCC RCC_APB2Periph_GPIOB
#define BSP_I2C_PORT GPIOB
#define GPIO_SDA GPIO_PIN_7
#define GPIO_SCL GPIO_PIN_6
#define BSP_I2C_WRITE 0x00
#define BSP_I2C_READ 0x01

#define SDA_IN()                                                               \
    do {                                                                       \
        GPIO_InitTypeDef GPIO_InitStruct = {0};                                \
        GPIO_InitStruct.Pin = GPIO_SDA;                                        \
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                \
        GPIO_InitStruct.Pull = GPIO_NOPULL;                                    \
        HAL_GPIO_Init(BSP_I2C_PORT, &GPIO_InitStruct);                         \
    } while (0)

#define SDA_OUT()                                                              \
    do {                                                                       \
        GPIO_InitTypeDef GPIO_InitStruct = {0};                                \
        GPIO_InitStruct.Pin = GPIO_SDA;                                        \
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                            \
        GPIO_InitStruct.Pull = GPIO_NOPULL;                                    \
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                          \
        HAL_GPIO_Init(BSP_I2C_PORT, &GPIO_InitStruct);                         \
    } while (0)

#define SDA_GET() HAL_GPIO_ReadPin(BSP_I2C_PORT, GPIO_SDA)
#define SDA(x)     HAL_GPIO_WritePin(BSP_I2C_PORT, GPIO_SDA, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define SCL(x)     HAL_GPIO_WritePin(BSP_I2C_PORT, GPIO_SCL, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))

void bsp_i2c_init(void);
void bsp_i2c_start(void);
void bsp_i2c_stop(void);
void bsp_i2c_send_ack(uint8_t ack);
uint8_t bsp_i2c_wait_ack(void);
void bsp_i2c_send_byte(uint8_t dat);
uint8_t bsp_i2c_read_byte(void);

#endif /* AXK_BSP_I2C_H */
