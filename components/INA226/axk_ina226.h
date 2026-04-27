#ifndef AXK_INA226_H
#define AXK_INA226_H

#include <stdint.h>
#include "axk_bsp_i2c.h"

#define AXK_INA226_ADDR_WRITE    0x80
#define AXK_INA226_ADDR_READ     0x81

#define AXK_INA226_REG_CONFIG    0x00
#define AXK_INA226_REG_SHUNT_V   0x01
#define AXK_INA226_REG_BUS_V     0x02
#define AXK_INA226_REG_POWER     0x03
#define AXK_INA226_REG_CURRENT   0x04
#define AXK_INA226_REG_CALIB     0x05
#define AXK_INA226_REG_MASK_EN   0x06
#define AXK_INA226_REG_ALERT     0x07
#define AXK_INA226_REG_MANUF_ID  0xFE
#define AXK_INA226_REG_DIE_ID    0xFF

#define AXK_INA226_MANUF_ID_VAL  0x5449
#define AXK_INA226_DIE_ID_VAL    0x2260

typedef struct {
    float r_shunt;
    float current_lsb;
} AxkIna226Device;

uint8_t axk_ina226_write_reg(uint8_t reg, uint16_t data);
uint8_t axk_ina226_read_reg(uint8_t reg, uint16_t *data);
uint8_t axk_ina226_init(AxkIna226Device *dev, float r_shunt, float max_current);
uint8_t axk_ina226_check_id(void);
float axk_ina226_get_bus_voltage(void);
float axk_ina226_get_shunt_voltage(void);
float axk_ina226_get_current(AxkIna226Device *dev);
float axk_ina226_get_power(AxkIna226Device *dev);

#endif /* AXK_INA226_H */
