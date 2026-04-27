#ifndef AXK_PWR_CTL_H
#define AXK_PWR_CTL_H

#include "emMCP.h"
#include "axk_ina226.h"

typedef struct {
    float target_v;
    float real_v;
    uint8_t is_output_on;
    uint8_t is_pps;
} AxkSysCtrl;

extern AxkSysCtrl g_axk_sys_ctrl;
extern AxkIna226Device g_axk_power_monitor;

void axk_pwr_handle_key_voltage(uint8_t key_v);
void axk_pwr_handle_serial_pps_change(float pps_val);
void axk_pwr_set_relay_handler(void *arg);
void axk_pwr_get_relay_handler(void *arg);
void axk_pwr_set_voltage_handler(void *arg);
void axk_pwr_get_voltage_handler(void *arg);
void axk_pwr_get_output_params_handler(void *arg);

#endif /* AXK_PWR_CTL_H */
