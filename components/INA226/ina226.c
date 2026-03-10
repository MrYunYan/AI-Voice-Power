/**
 * @file ina226.c
 * @brief INA226 驱动实现文件
 */

#include "ina226.h"

/* ---------------- I2C 底层通信封装 ---------------- */

/**
 * @brief  向 INA226 的指定寄存器写入一个 16 位的数据
 * @param  reg: 寄存器地址
 * @param  data: 要写入的 16 位数据
 * @return 0: 成功, 1: 失败
 */
uint8_t INA226_WriteReg(uint8_t reg, uint16_t data) {
    bsp_i2c_start();
    
    /* 1. 发送设备写地址 */
    bsp_i2c_send_byte(INA226_ADDR_WRITE);
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    /* 2. 发送内部寄存器地址 */
    bsp_i2c_send_byte(reg);
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    /* 3. 发送数据的高 8 位 (MSB First) */
    bsp_i2c_send_byte((uint8_t)(data >> 8));
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    /* 4. 发送数据的低 8 位 (LSB) */
    bsp_i2c_send_byte((uint8_t)(data & 0xFF));
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    bsp_i2c_stop();
    return 0; // 成功
}

/**
 * @brief  从 INA226 的指定寄存器读取一个 16 位的数据
 * @param  reg: 寄存器地址
 * @param  data: 用于保存读取到的数据的指针
 * @return 0: 成功, 1: 失败
 */
uint8_t INA226_ReadReg(uint8_t reg, uint16_t *data) {
    uint8_t data_h, data_l;
    
    /* 第一阶段：告诉芯片我们要读哪个寄存器 */
    bsp_i2c_start();
    bsp_i2c_send_byte(INA226_ADDR_WRITE);
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    bsp_i2c_send_byte(reg);
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    /* 第二阶段：重启 I2C 总线，开始读取数据 */
    bsp_i2c_start();
    bsp_i2c_send_byte(INA226_ADDR_READ);
    if (bsp_i2c_wait_ack() != 0) return 1;
    
    /* 先读高 8 位，并发送 ACK 表示还要继续读 */
    data_h = bsp_i2c_read_byte();
    bsp_i2c_send_ack(0); // 0 表示 ACK (根据你的I2C驱动，通常0位拉低表示ACK)
    
    /* 再读低 8 位，并发送 NACK 表示读完了 */
    data_l = bsp_i2c_read_byte();
    bsp_i2c_send_ack(1); // 1 表示 NACK
    
    bsp_i2c_stop();
    
    /* 组合高低 8 位 */
    *data = ((uint16_t)data_h << 8) | data_l;
    return 0;
}


/* ---------------- 核心功能实现 ---------------- */

/**
 * @brief  检查芯片连接是否正常 (读取制造商和芯片ID)
 * @return 0: 正常, 1: 异常
 */
uint8_t INA226_CheckID(void) {
    uint16_t manuf_id = 0;
    uint16_t die_id = 0;
    
    if (INA226_ReadReg(INA226_REG_MANUF_ID, &manuf_id) != 0) return 1;
    if (INA226_ReadReg(INA226_REG_DIE_ID, &die_id) != 0) return 1;
    
    if (manuf_id == INA226_MANUF_ID_VAL && die_id == INA226_DIE_ID_VAL) {
        return 0; // 匹配成功
    }
    return 1;
}

/**
 * @brief  初始化 INA226 并自动计算校准值
 * @param  dev: 设备结构体指针
 * @param  r_shunt: 实际电路中的采样电阻阻值 (如 0.01 欧姆)
 * @param  max_current: 项目预期的最大电流 (如 5.0 安培)
 * @return 0: 成功, 1: 失败
 */
uint8_t INA226_Init(INA226_Device_t *dev, float r_shunt, float max_current) {
    uint16_t config_value;
    uint16_t calib_value;
    
    /* 保存电阻值 */
    dev->r_shunt = r_shunt;
    
    /* 1. 确保硬件 I2C 引脚已初始化 */
    bsp_i2c_init();
    
    /* 2. 检查芯片是否存在 */
    if (INA226_CheckID() != 0) {
        return 1; // 找不到芯片，直接返回
    }
    
    /* 3. 软件复位芯片并配置工作模式
     * 默认上电配置为：0x4127 [cite: 744]
     * 我们这里可以配置平均采样次数等。为了简单，先使用默认参数：
     * 平均次数=1, 转换时间=1.1ms, 连续测量分流和总线电压。
     */
    config_value = 0x4127; 
    if (INA226_WriteReg(INA226_REG_CONFIG, config_value) != 0) return 1;
    
    /* 4. 计算 Current LSB 和 校准值 (Calibration) */
    /* 公式：Current_LSB = Max Expected Current / 32768 */
    dev->current_lsb = max_current / 32768.0f;
    
    /* 为了避免浮点数精度带来的微小误差，有时会让 LSB 向上取整为一个好算的数字。
     * 但这里为了通用性，直接使用精确计算。
     * 公式：Cal = 0.00512 / (Current_LSB * R_SHUNT) 
     */
    calib_value = (uint16_t)(0.00512f / (dev->current_lsb * dev->r_shunt));
    
    /* 写入校准寄存器 */
    if (INA226_WriteReg(INA226_REG_CALIB, calib_value) != 0) return 1;
    
    return 0; // 初始化成功
}

/* ---------------- 数据读取实现 ---------------- */

/**
 * @brief  获取总线电压 (负载端的电压)
 * @return 电压值，单位：伏特 (V)
 */
float INA226_GetBusVoltage(void) {
    uint16_t reg_val = 0;
    INA226_ReadReg(INA226_REG_BUS_V, &reg_val);
    
    /* 总线电压的 LSB 固定为 1.25 mV = 0.00125 V */
    return (float)reg_val * 0.00125f;
}

/**
 * @brief  获取分流电阻两端的电压差
 * @return 电压值，单位：伏特 (V)
 */
float INA226_GetShuntVoltage(void) {
    uint16_t reg_val = 0;
    int16_t signed_val = 0;
    
    INA226_ReadReg(INA226_REG_SHUNT_V, &reg_val);
    
    /* 分流电压可能为负数（反向电流），所以需要强制转换为有符号整数 */
    signed_val = (int16_t)reg_val;
    
    /* 分流电压的 LSB 固定为 2.5 uV = 0.0000025 V */
    return (float)signed_val * 0.0000025f;
}

/**
 * @brief  获取当前的电流值
 * @param  dev: 设备结构体指针 (用于获取计算好的 LSB)
 * @return 电流值，单位：安培 (A)
 */
float INA226_GetCurrent(INA226_Device_t *dev) {
    uint16_t reg_val = 0;
    int16_t signed_val = 0;
    
    INA226_ReadReg(INA226_REG_CURRENT, &reg_val);
    signed_val = (int16_t)reg_val; // 电流也可能有方向
    
    return (float)signed_val * dev->current_lsb;
}

/**
 * @brief  获取当前的功率值
 * @param  dev: 设备结构体指针
 * @return 功率值，单位：瓦特 (W)
 */
float INA226_GetPower(INA226_Device_t *dev) {
    uint16_t reg_val = 0;
    float power_lsb;
    
    INA226_ReadReg(INA226_REG_POWER, &reg_val);
    
    /* 功率寄存器的 LSB 内部被固定设定为电流 LSB 的 25 倍 */
    power_lsb = dev->current_lsb * 25.0f;
    
    return (float)reg_val * power_lsb;
}