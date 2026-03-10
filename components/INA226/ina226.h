/**
 * @file ina226.h
 * @brief INA226 电流电压监控芯片驱动头文件
 * @note 适用于各种 C 语言嵌入式平台，代码已做模块化处理。
 */

#ifndef __INA226_H__
#define __INA226_H__

#include <stdint.h>

/* 根据你的 I2C 驱动引入对应的头文件 */
#include "stm32f10x_bsp_i2c.h"

/* ---------------- 芯片 I2C 地址定义 ---------------- */
/* * 当 A0 和 A1 引脚都接地 (GND) 时，7位地址为 1000000 (0x40) [cite: 681]。
 * 左移 1 位后，写地址为 0x80，读地址为 0x81。
 */
#define INA226_ADDR_WRITE    0x80
#define INA226_ADDR_READ     0x81

/* ---------------- 寄存器地址定义 ---------------- */
#define INA226_REG_CONFIG    0x00 // 配置寄存器 [cite: 744]
#define INA226_REG_SHUNT_V   0x01 // 分流电压寄存器 [cite: 745]
#define INA226_REG_BUS_V     0x02 // 总线电压寄存器 [cite: 745]
#define INA226_REG_POWER     0x03 // 功率寄存器 [cite: 746]
#define INA226_REG_CURRENT   0x04 // 电流寄存器 [cite: 747]
#define INA226_REG_CALIB     0x05 // 校准寄存器 [cite: 750]
#define INA226_REG_MASK_EN   0x06 // 掩码/使能寄存器 [cite: 751]
#define INA226_REG_ALERT     0x07 // 报警限制寄存器 [cite: 752]
#define INA226_REG_MANUF_ID  0xFE // 制造商 ID 寄存器 [cite: 753]
#define INA226_REG_DIE_ID    0xFF // 芯片 ID 寄存器 [cite: 754]

/* ---------------- 芯片常量定义 ---------------- */
#define INA226_MANUF_ID_VAL  0x5449 // 固定的制造商 ID [cite: 753]
#define INA226_DIE_ID_VAL    0x2260 // 固定的芯片 ID [cite: 754]

/* ---------------- 配置结构体 ---------------- */
/* * 将 INA226 的参数封装在一个结构体中，方便在项目中管理多个芯片，
 * 或者记录传入的电阻值和计算得出的 LSB，方便后续计算。
 */
typedef struct {
    float r_shunt;        // 采样电阻的阻值，单位：欧姆 (Ohm)
    float current_lsb;    // 计算得出的电流 LSB，单位：安培/位 (A/bit)
} INA226_Device_t;

/* ---------------- 函数声明 ---------------- */
/* 基础寄存器读写接口 */
uint8_t INA226_WriteReg(uint8_t reg, uint16_t data);
uint8_t INA226_ReadReg(uint8_t reg, uint16_t *data);

/* 核心功能接口 */
uint8_t INA226_Init(INA226_Device_t *dev, float r_shunt, float max_current);
uint8_t INA226_CheckID(void);

/* 数据读取接口 */
float INA226_GetBusVoltage(void);
float INA226_GetShuntVoltage(void);
float INA226_GetCurrent(INA226_Device_t *dev);
float INA226_GetPower(INA226_Device_t *dev);

#endif /* __INA226_H__ */