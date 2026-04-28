#include "axk_bsp_i2c.h"
#include "stdio.h"
#include "stm32f1xx_hal_gpio.h"


/*
 * axk_bsp_i2c_init：初始化 I2C GPIO
 * @brief  配置 SDA 和 SCL 引脚为推挽输出模式
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_bsp_i2c_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = AXK_BSP_I2C_SDA_PIN | AXK_BSP_I2C_SCL_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(AXK_BSP_I2C_PORT, &GPIO_InitStructure);
}


/*
 * axk_bsp_i2c_start：发送 I2C 起始信号
 * @brief  产生 I2C 起始时序
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_bsp_i2c_start(void)
{
    AXK_BSP_I2C_SDA_OUT();
    AXK_BSP_I2C_SCL(1);
    AXK_BSP_I2C_SDA(0);

    AXK_BSP_I2C_SDA(1);
    AXK_DELAY_US(5);
    AXK_BSP_I2C_SDA(0);
    AXK_DELAY_US(5);

    AXK_BSP_I2C_SCL(0);
}


/*
 * axk_bsp_i2c_stop：发送 I2C 停止信号
 * @brief  产生 I2C 停止时序
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_bsp_i2c_stop(void)
{
    AXK_BSP_I2C_SDA_OUT();
    AXK_BSP_I2C_SCL(0);
    AXK_BSP_I2C_SDA(0);

    AXK_BSP_I2C_SCL(1);
    AXK_DELAY_US(5);
    AXK_BSP_I2C_SDA(1);
    AXK_DELAY_US(5);
}


/*
 * axk_bsp_i2c_send_ack：发送应答信号
 * @brief  向从设备发送 ACK 或 NACK
 * @param  ack：0 表示 ACK，1 表示 NACK
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_bsp_i2c_send_ack(uint8_t ack)
{
    AXK_BSP_I2C_SDA_OUT();
    AXK_BSP_I2C_SCL(0);
    AXK_BSP_I2C_SDA(0);
    AXK_DELAY_US(5);
    AXK_BSP_I2C_SDA(ack);
    AXK_BSP_I2C_SCL(1);
    AXK_DELAY_US(5);
    AXK_BSP_I2C_SCL(0);
    AXK_BSP_I2C_SDA(1);
}


/*
 * axk_bsp_i2c_wait_ack：等待从设备应答
 * @brief  检测从设备是否发送 ACK
 * @param  无
 * @return 0：应答成功；1：应答失败
 * @note   超时计数器为 10 次，每次 5us
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_bsp_i2c_wait_ack(void)
{
    char ack = 0;
    unsigned char ack_flag = 10;

    AXK_BSP_I2C_SCL(0);
    AXK_BSP_I2C_SDA(1);
    AXK_BSP_I2C_SDA_IN();

    AXK_BSP_I2C_SCL(1);
    while ((AXK_BSP_I2C_SDA_GET() == 1) && (ack_flag)) {
        ack_flag--;
        AXK_DELAY_US(5);
    }

    if (ack_flag <= 0) {
        axk_bsp_i2c_stop();
        return 1;
    } else {
        AXK_BSP_I2C_SCL(0);
        AXK_BSP_I2C_SDA_OUT();
    }

    return ack;
}


/*
 * axk_bsp_i2c_send_byte：发送 I2C 字节数据
 * @brief  通过 SDA 发送 8 位数据，MSB 优先
 * @param  dat：要发送的字节数据
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_bsp_i2c_send_byte(uint8_t dat)
{
    uint8_t i = 0;

    AXK_BSP_I2C_SDA_OUT();
    AXK_BSP_I2C_SCL(0);

    for (i = 0; i < 8; i++) {
        AXK_BSP_I2C_SDA((dat & 0x80) >> 7);
        AXK_DELAY_US(1);
        AXK_BSP_I2C_SCL(1);
        AXK_DELAY_US(5);
        AXK_BSP_I2C_SCL(0);
        AXK_DELAY_US(5);
        dat <<= 1;
    }
}


/*
 * axk_bsp_i2c_read_byte：接收 I2C 字节数据
 * @brief  通过 SDA 接收 8 位数据，MSB 优先
 * @param  无
 * @return 接收到的字节数据
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_bsp_i2c_read_byte(void)
{
    unsigned char i = 0;
    unsigned char receive = 0;

    AXK_BSP_I2C_SDA_IN();

    for (i = 0; i < 8; i++) {
        AXK_BSP_I2C_SCL(0);
        AXK_DELAY_US(5);
        AXK_BSP_I2C_SCL(1);
        AXK_DELAY_US(5);
        receive <<= 1;

        if (AXK_BSP_I2C_SDA_GET()) {
            receive |= 1;
        }

        AXK_DELAY_US(5);
    }

    AXK_BSP_I2C_SCL(0);

    return receive;
}
