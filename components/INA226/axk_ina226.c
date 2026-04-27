#include "axk_ina226.h"


/*
 * axk_ina226_write_reg：向 INA226 指定寄存器写入 16 位数据
 * @brief  通过 I2C 向 INA226 写入寄存器数据
 * @param  reg：寄存器地址
 * @param  data：要写入的 16 位数据
 * @return 0：成功；1：失败
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_ina226_write_reg(uint8_t reg, uint16_t data)
{
    bsp_i2c_start();

    bsp_i2c_send_byte(AXK_INA226_ADDR_WRITE);
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_send_byte(reg);
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_send_byte((uint8_t)(data >> 8));
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_send_byte((uint8_t)(data & 0xFF));
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_stop();

    return 0;
}


/*
 * axk_ina226_read_reg：从 INA226 指定寄存器读取 16 位数据
 * @brief  通过 I2C 从 INA226 读取寄存器数据
 * @param  reg：寄存器地址
 * @param  data：用于保存读取数据的指针
 * @return 0：成功；1：失败
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_ina226_read_reg(uint8_t reg, uint16_t *data)
{
    uint8_t data_h = 0;
    uint8_t data_l = 0;

    bsp_i2c_start();
    bsp_i2c_send_byte(AXK_INA226_ADDR_WRITE);
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_send_byte(reg);
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    bsp_i2c_start();
    bsp_i2c_send_byte(AXK_INA226_ADDR_READ);
    if (bsp_i2c_wait_ack() != 0) {
        return 1;
    }

    data_h = bsp_i2c_read_byte();
    bsp_i2c_send_ack(0);

    data_l = bsp_i2c_read_byte();
    bsp_i2c_send_ack(1);

    bsp_i2c_stop();

    *data = ((uint16_t)data_h << 8) | data_l;

    return 0;
}


/*
 * axk_ina226_check_id：检查芯片连接是否正常
 * @brief  读取制造商 ID 和芯片 ID 验证连接
 * @param  无
 * @return 0：正常；1：异常
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_ina226_check_id(void)
{
    uint16_t manuf_id = 0;
    uint16_t die_id = 0;

    if (axk_ina226_read_reg(AXK_INA226_REG_MANUF_ID, &manuf_id) != 0) {
        return 1;
    }

    if (axk_ina226_read_reg(AXK_INA226_REG_DIE_ID, &die_id) != 0) {
        return 1;
    }

    if (manuf_id == AXK_INA226_MANUF_ID_VAL && die_id == AXK_INA226_DIE_ID_VAL) {
        return 0;
    }

    return 1;
}


/*
 * axk_ina226_init：初始化 INA226 并自动计算校准值
 * @brief  配置 INA226 工作模式和校准寄存器
 * @param  dev：设备结构体指针
 * @param  r_shunt：采样电阻阻值，单位：欧姆
 * @param  max_current：预期最大电流，单位：安培
 * @return 0：成功；1：失败
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
uint8_t axk_ina226_init(AxkIna226Device *dev, float r_shunt, float max_current)
{
    uint16_t config_value = 0;
    uint16_t calib_value = 0;

    if (dev == NULL) {
        return 1;
    }

    dev->r_shunt = r_shunt;
    bsp_i2c_init();

    if (axk_ina226_check_id() != 0) {
        return 1;
    }

    config_value = 0x4127;
    if (axk_ina226_write_reg(AXK_INA226_REG_CONFIG, config_value) != 0) {
        return 1;
    }

    dev->current_lsb = max_current / 32768.0f;
    calib_value = (uint16_t)(0.00512f / (dev->current_lsb * dev->r_shunt));

    if (axk_ina226_write_reg(AXK_INA226_REG_CALIB, calib_value) != 0) {
        return 1;
    }

    return 0;
}


/*
 * axk_ina226_get_bus_voltage：获取总线电压
 * @brief  读取 INA226 总线电压寄存器
 * @param  无
 * @return 电压值，单位：伏特 (V)
 * @note   总线电压 LSB 固定为 1.25mV
 * @author Master_Yang
 * @date   2026-04-27
 */
float axk_ina226_get_bus_voltage(void)
{
    uint16_t reg_val = 0;

    axk_ina226_read_reg(AXK_INA226_REG_BUS_V, &reg_val);

    return (float)reg_val * 0.00125f;
}


/*
 * axk_ina226_get_shunt_voltage：获取分流电阻电压差
 * @brief  读取 INA226 分流电压寄存器
 * @param  无
 * @return 电压值，单位：伏特 (V)
 * @note   分流电压可能为负数
 * @author Master_Yang
 * @date   2026-04-27
 */
float axk_ina226_get_shunt_voltage(void)
{
    uint16_t reg_val = 0;
    int16_t signed_val = 0;

    axk_ina226_read_reg(AXK_INA226_REG_SHUNT_V, &reg_val);
    signed_val = (int16_t)reg_val;

    return (float)signed_val * 0.0000025f;
}


/*
 * axk_ina226_get_current：获取当前电流值
 * @brief  读取 INA226 电流寄存器并转换为实际电流
 * @param  dev：设备结构体指针，用于获取 current_lsb
 * @return 电流值，单位：安培 (A)
 * @note   电流可能有方向
 * @author Master_Yang
 * @date   2026-04-27
 */
float axk_ina226_get_current(AxkIna226Device *dev)
{
    uint16_t reg_val = 0;
    int16_t signed_val = 0;

    if (dev == NULL) {
        return 0.0f;
    }

    axk_ina226_read_reg(AXK_INA226_REG_CURRENT, &reg_val);
    signed_val = (int16_t)reg_val;

    return (float)signed_val * dev->current_lsb;
}


/*
 * axk_ina226_get_power：获取当前功率值
 * @brief  读取 INA226 功率寄存器并转换为实际功率
 * @param  dev：设备结构体指针，用于获取 current_lsb
 * @return 功率值，单位：瓦特 (W)
 * @note   功率 LSB 为 current_lsb 的 25 倍
 * @author Master_Yang
 * @date   2026-04-27
 */
float axk_ina226_get_power(AxkIna226Device *dev)
{
    uint16_t reg_val = 0;
    float power_lsb = 0.0f;

    if (dev == NULL) {
        return 0.0f;
    }

    axk_ina226_read_reg(AXK_INA226_REG_POWER, &reg_val);
    power_lsb = dev->current_lsb * 25.0f;

    return (float)reg_val * power_lsb;
}
