#include "axk_ch224.h"
#include "log.h"
#include <stdint.h>


/*
 * axk_ch224_init：初始化CH224 I2C
 * @brief  初始化I2C外设并检测CH224是否响应
 * @param  无
 * @return 0：成功；非0：失败
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_init(void)
{
    int ack;

    AXK_CH224_I2C_ACLL(init);
    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);
    ack = (int)AXK_CH224_I2C_ACLL(wait_ack);
    AXK_CH224_I2C_ACLL(stop);

    return ack;
}


/*
 * axk_ch224_get_status：获取CH224状态寄存器
 * @brief  读取CH224指定寄存器的值
 * @param  only_read_reg：要读取的寄存器地址
 * @return 成功返回状态寄存器值，失败返回错误码：-1设备无响应，-2发送寄存器地址失败，-3重复起始后设备无响应，-4寄存器不是只读寄存器
 * @note   仅支持读取STATUS等只读寄存器
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_get_status(AxkCh224Reg only_read_reg)
{
    unsigned char status = 0;

    if (only_read_reg == AXK_CH224_REG_VOUT ||
        only_read_reg == AXK_CH224_REG_AVS_LSB ||
        only_read_reg == AXK_CH224_REG_AVS_MSB ||
        only_read_reg == AXK_CH224_REG_PPS) {
        return -4;
    }

    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -1;
    }

    AXK_CH224_I2C_ACLL(send_byte, only_read_reg);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -2;
    }

    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_READ_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -3;
    }

    status = AXK_CH224_I2C_ACLL(read_byte);
    AXK_CH224_I2C_ACLL(send_ack, AXK_CH224_NACK);
    AXK_CH224_I2C_ACLL(stop);

    return (int)status;
}


/*
 * axk_ch224_set_vout：设置CH224电压寄存器
 * @brief  通过I2C向CH224写入固定档位电压值
 * @param  vout：输出电压档位枚举值
 * @return 0：成功；负值：失败码
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_set_vout(AxkCh224Vout vout)
{
    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -1;
    }

    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_REG_VOUT);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -2;
    }

    AXK_CH224_I2C_ACLL(send_byte, vout);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -3;
    }

    AXK_CH224_I2C_ACLL(stop);

    return 0;
}


/*
 * axk_ch224_set_mode：设置CH224 PPS/AVS模式
 * @brief  配置CH224进入PPS或AVS模式
 * @param  mode：模式枚举值，AXK_CH224_VOUT_PPS 或 AXK_CH224_VOUT_AVS
 * @return 0：成功；负值：失败码
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_set_mode(AxkCh224Vout mode)
{
    if (mode != AXK_CH224_VOUT_PPS && mode != AXK_CH224_VOUT_AVS) {
        return -4;
    }

    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -1;
    }

    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_REG_VOUT);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -2;
    }

    AXK_CH224_I2C_ACLL(send_byte, mode);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -3;
    }

    AXK_CH224_I2C_ACLL(stop);

    return 0;
}


/*
 * axk_ch224_set_pps_vout：设置CH224 PPS模式输出电压
 * @brief  通过I2C向CH224写入PPS电压寄存器
 * @param  pps_vout：PPS模式输出电压值，范围 5.0~28.0 V
 * @return 0：成功；负值：失败码；-10：电压值超出范围
 * @note   电压值将乘以10后写入寄存器
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_set_pps_vout(float pps_vout)
{
    if (pps_vout < 5.0f || pps_vout > 28.0f) {
        return -10;
    }

    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -1;
    }

    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_REG_PPS);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -4;
    }

    unsigned char pps_val = (unsigned char)(pps_vout * 10.0f);
    AXK_CH224_I2C_ACLL(send_byte, pps_val);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -5;
    }

    AXK_CH224_I2C_ACLL(stop);

    return 0;
}


/*
 * axk_ch224_set_avs_vout：设置CH224 AVS模式输出电压
 * @brief  通过I2C向CH224写入AVS电压寄存器
 * @param  avs_vout：AVS模式输出电压值，范围 5.0~28.0 V
 * @return 0：成功；负值：失败码；-10：电压值超出范围
 * @note   电压值分为MSB和LSB分别写入
 * @author Master_Yang
 * @date   2026-04-27
 */
int axk_ch224_set_avs_vout(float avs_vout)
{
    if (avs_vout < 5.0f || avs_vout > 28.0f) {
        return -10;
    }

    unsigned int avs_val = (unsigned int)(avs_vout * 10.0f);
    unsigned char avs_lsb = avs_val & 0x00FF;
    unsigned char avs_msb = ((avs_val >> 8) & 0xFF) | 0x80;

    AXK_CH224_I2C_ACLL(start);
    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_I2C_ADDR << 1 | AXK_CH224_WRITE_CMD);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -1;
    }

    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_REG_AVS_LSB);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -3;
    }

    AXK_CH224_I2C_ACLL(send_byte, avs_lsb);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -3;
    }

    AXK_CH224_I2C_ACLL(send_byte, AXK_CH224_REG_AVS_MSB);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -2;
    }

    AXK_CH224_I2C_ACLL(send_byte, avs_msb);

    if (AXK_CH224_I2C_ACLL(wait_ack)) {
        AXK_CH224_I2C_ACLL(stop);
        return -4;
    }

    AXK_CH224_I2C_ACLL(stop);

    return 0;
}
