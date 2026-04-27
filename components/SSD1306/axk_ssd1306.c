/*
 * @file    axk_ssd1306.c
 * @brief   SSD1306 OLED 驱动源文件
 * @version V1.0
 * @date    2026-04-27
 * @author  Master_Yang
 * @license 
 * @note    
 */

#include "axk_ssd1306.h"
#include "gt20l16s/axk_gt20l16s.h"
#include <stddef.h>

#if __has_include("axk_ssd1306_font.h")
#include "axk_ssd1306_font.h"
#else
#error "axk_ssd1306_font.h not found"
#endif

/* ==================================================================
 * 全局变量
 * ================================================================== */
unsigned int  g_axk_ssd1306_font_addr = 0;
unsigned char g_axk_oled_gram[144][8];

/* ==================================================================
 * 本地函数声明
 * ================================================================== */
static unsigned short int axk_ssd1306_pow(unsigned char m, unsigned char n);
static void axk_ssd1306_display_data_form_font_chip(unsigned char x,
                                                     unsigned char y,
                                                     unsigned char *dp);
static void axk_ssd1306_display_data_form_font_chip_8x16(unsigned char x,
                                                          unsigned char y,
                                                          unsigned char *dp);
static void axk_ssd1306_clear_area(char *str, int max_len);

/* ==================================================================
 * 函数实现
 * ================================================================== */

/*
 * axk_ssd1306_write_byte：向 SSD1306 写入一个字节
 * @brief  通过 SPI/I2C 接口写入数据或命令
 * @param  data：要写入的数据
 * @param  cmd：AXK_SSD1306_WRITE_CMD (0) 写命令，AXK_SSD1306_WRITE_DATA (1) 写数据
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_write_byte(unsigned char data, unsigned char cmd)
{
    if (cmd) {
#ifdef AXK_SSD1306_SPI
        AXK_SSD1306_ACLL(dc_set);
#elif AXK_SSD1306_I2C
#endif
    } else {
#ifdef AXK_SSD1306_SPI
        AXK_SSD1306_ACLL(dc_reset);
#elif AXK_SSD1306_I2C
#endif
    }

#ifdef AXK_SSD1306_SPI
    AXK_SSD1306_ACLL(cs_reset);
    AXK_SSD1306_ACLL(transmit, data, 100);
    AXK_SSD1306_ACLL(cs_set);
    AXK_SSD1306_ACLL(dc_set);
#elif AXK_SSD1306_I2C
#endif
}


/*
 * axk_ssd1306_set_color_turn：设置反色显示
 * @brief  设置 OLED 颜色翻转模式
 * @param  i：0 正常显示，1 反色显示
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_set_color_turn(unsigned char i)
{
    if (i == 0) {
#ifdef AXK_SSD1306_SPI
        axk_ssd1306_write_byte(0xA6, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
    } else if (i == 1) {
#ifdef AXK_SSD1306_SPI
        axk_ssd1306_write_byte(0xA7, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
    }
}


/*
 * axk_ssd1306_set_display_turn：设置屏幕旋转
 * @brief  设置 OLED 扫描方向，实现 180 度旋转
 * @param  i：0 正常方向，1 旋转 180 度
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_set_display_turn(unsigned char i)
{
    if (i == 0) {
#ifdef AXK_SSD1306_SPI
        axk_ssd1306_write_byte(0xC8, AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0xA1, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
    } else if (i == 1) {
#ifdef AXK_SSD1306_SPI
        axk_ssd1306_write_byte(0xC0, AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0xA0, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
    }
}


/*
 * axk_ssd1306_open_display：开启 OLED 显示
 * @brief  打开电荷泵并开启显示
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_open_display(void)
{
#ifdef AXK_SSD1306_SPI
    axk_ssd1306_write_byte(0x8D, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x14, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xAF, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
}


/*
 * axk_ssd1306_close_display：关闭 OLED 显示
 * @brief  关闭电荷泵并进入休眠模式
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_close_display(void)
{
#ifdef AXK_SSD1306_SPI
    axk_ssd1306_write_byte(0x8D, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x10, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xAE, AXK_SSD1306_WRITE_CMD);
#elif AXK_SSD1306_I2C
#endif
}


/*
 * axk_ssd1306_refresh：刷新 OLED 显示
 * @brief  将显存数据刷新到 OLED 屏幕
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_refresh(void)
{
    unsigned char i, n;

    for (i = 0; i < 8; i++) {
        axk_ssd1306_write_byte(0xB0 + i, AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0x00, AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0x10, AXK_SSD1306_WRITE_CMD);
        for (n = 0; n < 128; n++) {
            axk_ssd1306_write_byte(g_axk_oled_gram[n][i], AXK_SSD1306_WRITE_DATA);
        }
    }
}


/*
 * axk_ssd1306_rolling：滚动显示
 * @brief  预留接口，当前为空实现
 * @param  无
 * @return 无
 * @note   待后续实现
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_rolling(void)
{
}


/*
 * axk_ssd1306_clear_screen：清除屏幕内容
 * @brief  将显存全部清零并刷新显示
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_clear_screen(void)
{
    unsigned char i, n;

    for (i = 0; i < 8; i++) {
        for (n = 0; n < 128; n++) {
            g_axk_oled_gram[n][i] = 0x00;
        }
    }
    axk_ssd1306_refresh();
}


/*
 * axk_ssd1306_set_address：设置写入数据的起始地址
 * @brief  设置 OLED 显存的页地址和列地址
 * @param  x：列地址 (0-127)
 * @param  y：页地址 (0-7)
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_set_address(unsigned char x, unsigned char y)
{
    axk_ssd1306_write_byte(0xB0 + y, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(((x & 0xF0) >> 4) | 0x10, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte((x & 0x0F), AXK_SSD1306_WRITE_CMD);
}


/*
 * axk_ssd1306_draw_point：绘制一个点
 * @brief  在指定坐标绘制或清除一个像素点
 * @param  x：列坐标 (0-127)
 * @param  y：行坐标 (0-63)
 * @param  draw_mode：0 设置像素，1 清除像素（反显逻辑）
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_draw_point(unsigned char x, unsigned char y,
                            unsigned char draw_mode)
{
    unsigned char i, m, n;

    i = y / 8;
    m = y % 8;
    n = 1 << m;

    if (!draw_mode) {
        g_axk_oled_gram[x][i] |= n;
    } else {
        g_axk_oled_gram[x][i] = ~g_axk_oled_gram[x][i];
        g_axk_oled_gram[x][i] |= n;
        g_axk_oled_gram[x][i] = ~g_axk_oled_gram[x][i];
    }
}


/*
 * axk_ssd1306_draw_line：绘制一条直线
 * @brief  使用 Bresenham 算法绘制两点之间的直线
 * @param  x1：起点列坐标 (0-127)
 * @param  y1：起点行坐标 (0-63)
 * @param  x2：终点列坐标 (0-127)
 * @param  y2：终点行坐标 (0-63)
 * @param  draw_mode：绘制模式
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_draw_line(unsigned char x1, unsigned char y1, unsigned char x2,
                           unsigned char y2, unsigned char draw_mode)
{
    unsigned int t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0) {
        incx = 1;
    } else if (delta_x == 0) {
        incx = 0;
    } else {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;
    } else {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y) {
        distance = delta_x;
    } else {
        distance = delta_y;
    }

    for (t = 0; t < distance + 1; t++) {
        axk_ssd1306_draw_point(uRow, uCol, draw_mode);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}


/*
 * axk_ssd1306_draw_circle：绘制一个圆
 * @brief  使用中点圆算法绘制圆形
 * @param  x0：圆心列坐标 (0-127)
 * @param  y0：圆心行坐标 (0-63)
 * @param  r：半径
 * @param  draw_mode：绘制模式 (0: 反显, 1: 正常)
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_draw_circle(unsigned char x0, unsigned char y0,
                             unsigned char r, unsigned char draw_mode)
{
    if (x0 >= 128 || y0 >= 64 || (y0 + 2) > 8 || r >= 128) {
        return;
    }

    int a, b, num;

    a = 0;
    b = r;
    while (2 * b * b >= r * r) {
        axk_ssd1306_draw_point(x0 + a, y0 - b, draw_mode);
        axk_ssd1306_draw_point(x0 - a, y0 - b, draw_mode);
        axk_ssd1306_draw_point(x0 - a, y0 + b, draw_mode);
        axk_ssd1306_draw_point(x0 + a, y0 + b, draw_mode);
        axk_ssd1306_draw_point(x0 + b, y0 + a, draw_mode);
        axk_ssd1306_draw_point(x0 + b, y0 - a, draw_mode);
        axk_ssd1306_draw_point(x0 - b, y0 - a, draw_mode);
        axk_ssd1306_draw_point(x0 - b, y0 + a, draw_mode);
        a++;
        num = (a * a + b * b) - r * r;
        if (num > 0) {
            b--;
            a--;
        }
    }
}


/*
 * axk_ssd1306_show_image_128x64：显示 128x64 全屏图像
 * @brief  将 128x64 点阵图像数据刷新到屏幕
 * @param  dp：图像数据指针
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_image_128x64(unsigned char *dp)
{
    unsigned char i, j;

    for (i = 0; i < 8; i++) {
        axk_ssd1306_set_address(0, i);
        for (j = 0; j < 128; j++) {
            axk_ssd1306_write_byte(*dp, AXK_SSD1306_WRITE_DATA);
            dp++;
        }
    }
}


/*
 * axk_ssd1306_show_char：显示字符
 * @brief  在指定位置显示单个 ASCII 字符
 * @param  x：起始列 (0-127)
 * @param  y：起始页 (0-7)
 * @param  font_size：字体大小枚举
 * @param  show_mode：显示模式 (0: 正常, 1: 反色)
 * @param  c：要显示的字符
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_char(unsigned char x, unsigned char y,
                           AxkFontSize font_size, unsigned char show_mode,
                           unsigned char c)
{
    unsigned char i, m, temp, size, chr1;
    unsigned char x0 = x, y0 = y;

    if (font_size == AXK_FONT_SIZE_8) {
        size = 6;
    } else {
        size = (font_size / 8 + ((font_size % 8) ? 1 : 0)) * (font_size / 2);
    }

    chr1 = c - ' ';
    for (i = 0; i < size; i++) {
        if (font_size == AXK_FONT_SIZE_8) {
            temp = axk_asc2_0806[chr1][i];
        } else if (font_size == AXK_FONT_SIZE_12) {
            temp = axk_asc2_1206[chr1][i];
        } else if (font_size == AXK_FONT_SIZE_16) {
            temp = axk_asc2_1608[chr1][i];
        } else if (font_size == AXK_FONT_SIZE_24) {
            temp = axk_asc2_2412[chr1][i];
        } else {
            return;
        }

        for (m = 0; m < 8; m++) {
            if (temp & 0x01) {
                axk_ssd1306_draw_point(x, y, show_mode);
            } else {
                axk_ssd1306_draw_point(x, y, !show_mode);
            }
            temp >>= 1;
            y++;
        }
        x++;
        if ((font_size != AXK_FONT_SIZE_8) && ((x - x0) == font_size / 2)) {
            x = x0;
            y0 = y0 + 8;
        }
        y = y0;
    }
}


/*
 * axk_ssd1306_show_str：显示字符串
 * @brief  在指定位置显示 ASCII 字符串
 * @param  x：起始列 (0-127)
 * @param  y：起始页 (0-7)
 * @param  font_size：字体大小枚举
 * @param  show_mode：显示模式 (0: 正常, 1: 反色)
 * @param  str：字符串指针
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_str(unsigned char x, unsigned char y,
                          AxkFontSize font_size, unsigned char show_mode,
                          unsigned char *str)
{
    while ((*str >= ' ') && (*str <= '~')) {
        axk_ssd1306_show_char(x, y, font_size, show_mode, *str);
        if (font_size == AXK_FONT_SIZE_8) {
            x += 6;
        } else {
            x += font_size / 2;
        }
        str++;
    }
}


/*
 * axk_ssd1306_pow：计算 m 的 n 次方
 * @brief  辅助函数，用于数字显示时的位权计算
 * @param  m：底数
 * @param  n：指数
 * @return m 的 n 次方结果
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static unsigned short int axk_ssd1306_pow(unsigned char m, unsigned char n)
{
    unsigned short int result = 1;

    while (n--) {
        result *= m;
    }
    return result;
}


/*
 * axk_ssd1306_show_numble：显示数字
 * @brief  在指定位置显示 0-65535 范围内的数字
 * @param  x：起始列 (0-127)
 * @param  y：起始页 (0-7)
 * @param  font_size：字体大小枚举
 * @param  show_mode：显示模式 (0: 正常, 1: 反色)
 * @param  num：要显示的数字
 * @return 无
 * @note   固定显示 4 位数字，不足补零
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_numble(unsigned char x, unsigned char y,
                             AxkFontSize font_size, unsigned char show_mode,
                             unsigned int num)
{
    unsigned char t, temp, m = 0;

    if (font_size == AXK_FONT_SIZE_8) {
        m = 2;
    }
    for (t = 0; t < 4; t++) {
        temp = (num / axk_ssd1306_pow(10, 4 - t - 1)) % 10;
        if (temp == 0) {
            axk_ssd1306_show_char(x + (font_size / 2 + m) * t, y, font_size,
                                  show_mode, '0');
        } else {
            axk_ssd1306_show_char(x + (font_size / 2 + m) * t, y, font_size,
                                  show_mode, temp + '0');
        }
    }
}


/*
 * axk_ssd1306_init：初始化 SSD1306 OLED 显示屏
 * @brief  发送初始化命令序列，配置屏幕参数并清屏
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_init(void)
{
    if (AXK_SSD1306_ACLL(init) != AXK_SSD1306_SPI_READY) {
        return;
    }
    axk_ssd1306_write_byte(0xAE, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x00, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x10, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x40, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x81, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xCF, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xA1, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xC8, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xA6, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xA8, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x3F, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xD3, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x00, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xD5, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x80, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xD9, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xF1, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xDA, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x12, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xDB, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x40, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x20, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x02, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x8D, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0x14, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xA4, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_write_byte(0xA6, AXK_SSD1306_WRITE_CMD);
    axk_ssd1306_clear_screen();
    axk_ssd1306_write_byte(0xAF, AXK_SSD1306_WRITE_CMD);
}


#ifdef AXK_GT20L16_IS_AVAILABLE
/* ==================================================================
 * 字库芯片相关函数
 * ================================================================== */
unsigned int fontaddr = 0;

/*
 * axk_ssd1306_display_data_form_font_chip：从字库芯片读取 16x16 数据并显示
 * @brief  将 16x16 点阵数据显示到 OLED
 * @param  x：起始列
 * @param  y：起始页
 * @param  dp：点阵数据指针
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_ssd1306_display_data_form_font_chip(unsigned char x,
                                                     unsigned char y,
                                                     unsigned char *dp)
{
    if (x >= 128 || y >= 64 || (y + 2) > 8) {
        return;
    }

    unsigned char i, j;
    unsigned char current_page = y;

    for (j = 0; j < 2; j++) {
        unsigned char start_col = x;
        unsigned char end_col = x + 15;

        if (end_col >= 128) {
            end_col = 128 - 1;
        }
        if (start_col > end_col) {
            current_page++;
            dp += 16;
            continue;
        }

        axk_ssd1306_write_byte(0xB0 + current_page, AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0x10 | ((start_col >> 4) & 0x0F),
                               AXK_SSD1306_WRITE_CMD);
        axk_ssd1306_write_byte(0x00 | (start_col & 0x0F), AXK_SSD1306_WRITE_CMD);

        unsigned char draw_cols = end_col - start_col + 1;
        for (i = 0; i < draw_cols; i++) {
            axk_ssd1306_write_byte(dp[i], AXK_SSD1306_WRITE_DATA);
        }

        current_page++;
        dp += 16;
    }
}


/*
 * axk_ssd1306_display_data_form_font_chip_8x16：从字库芯片读取 8x16 数据并显示
 * @brief  将 8x16 点阵数据显示到 OLED
 * @param  x：起始列
 * @param  y：起始页
 * @param  dp：点阵数据指针
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_ssd1306_display_data_form_font_chip_8x16(unsigned char x,
                                                          unsigned char y,
                                                          unsigned char *dp)
{
    unsigned char n, m;

    for (n = 0; n < 2; n++) {
        axk_ssd1306_set_address(x, y);
        for (m = 0; m < 8; m++) {
            axk_ssd1306_write_byte(*dp, AXK_SSD1306_WRITE_DATA);
            dp++;
        }
        y++;
    }
}


/*
 * axk_ssd1306_show_gb2312_str：显示 GB2312 字符串
 * @brief  使用字库芯片显示 GB2312 编码的中文字符串
 * @param  x：起始列
 * @param  y：起始页
 * @param  str：GB2312 字符串指针
 * @return 无
 * @note   需要外部字库芯片支持
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_gb2312_str(unsigned char x, unsigned char y, char *str)
{
    unsigned char i = 0;
    unsigned char addr_high, addr_mid, addr_low;
    unsigned char fontbuf[32];

    while (str[i] > 0x00) {
        if ((str[i] >= 0xB0) && (str[i] <= 0xF7) && (str[i + 1] >= 0xA1)) {
            fontaddr = (str[i] - 0xB0) * 94;
            fontaddr += (str[i + 1] - 0xA1) + 846;
            fontaddr = fontaddr * 32;
            addr_high = (fontaddr & 0xFF0000) >> 16;
            addr_mid  = (fontaddr & 0xFF00) >> 8;
            addr_low  = (fontaddr & 0xFF);

            axk_gt20l16s_get_fonts_form_rom(addr_high, addr_mid, addr_low, fontbuf, 32);
            axk_ssd1306_display_data_form_font_chip(x, y, fontbuf);
            x += 16;
            i += 2;
        } else if ((str[i] >= 0xA1) && (str[i] <= 0xA3) && (str[i + 1] >= 0xA1)) {
            fontaddr = (str[i] - 0xA1) * 94;
            fontaddr += (str[i + 1] - 0xA1);
            fontaddr = fontaddr * 32;

            addr_high = (fontaddr & 0xFF0000) >> 16;
            addr_mid  = (fontaddr & 0xFF00) >> 8;
            addr_low  = (fontaddr & 0xFF);

            axk_gt20l16s_get_fonts_form_rom(addr_high, addr_mid, addr_low, fontbuf, 32);
            axk_ssd1306_display_data_form_font_chip(x, y, fontbuf);
            x += 16;
            i += 2;
        } else if ((str[i] >= 0x20) && (str[i] <= 0x7E)) {
            unsigned char fontbuf_asc[16];

            fontaddr = (str[i] - 0x20);
            fontaddr = (unsigned long)(fontaddr * 16);
            fontaddr = (unsigned long)(fontaddr + 0x3CF80);

            addr_high = (fontaddr & 0xFF0000) >> 16;
            addr_mid  = (fontaddr & 0xFF00) >> 8;
            addr_low  = fontaddr & 0xFF;

            axk_gt20l16s_get_fonts_form_rom(addr_high, addr_mid, addr_low, fontbuf_asc, 16);
            axk_ssd1306_display_data_form_font_chip_8x16(x, y, fontbuf_asc);

            x += 8;
            i += 1;
        } else {
            i++;
        }
    }
}


/*
 * axk_ssd1306_show_utf8_str：显示 UTF-8 字符串
 * @brief  将 UTF-8 编码字符串转换为 GB2312 后显示
 * @param  x：起始列
 * @param  y：起始页
 * @param  str：UTF-8 字符串指针
 * @return 无
 * @note   需要外部字库芯片支持
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_utf8_str(unsigned char x, unsigned char y,
                               const char *str)
{
    unsigned char i = 0;
    unsigned char byte_len;
    static unsigned char fontbuf[2];
    static unsigned char _fontbuf[16];

    while (str[i] != '\0') {
        if ((str[i] >= 0x20) && (str[i] <= 0x7E)) {
            unsigned char addr_high, addr_mid, addr_low;
            unsigned int font_addr;

            font_addr = (str[i] - 0x20);
            font_addr = (unsigned long)(font_addr * 16);
            font_addr = (unsigned long)(font_addr + 0x3CF80);

            addr_high = (font_addr & 0xFF0000) >> 16;
            addr_mid  = (font_addr & 0xFF00) >> 8;
            addr_low  = font_addr & 0xFF;

            axk_gt20l16s_get_fonts_form_rom(addr_high, addr_mid, addr_low, _fontbuf, 16);
            axk_ssd1306_display_data_form_font_chip_8x16(x, y, _fontbuf);

            x += 8;
            i += 1;
        } else if (isStrUTF8(&str[i], 3)) {
            unsigned int unicode = utf8_to_unicode(&str[i], &byte_len);
            if (unicode == 0) {
                i++;
                continue;
            }
            unsigned int font_addr = unicode_to_gb2312_fontaddr(unicode);
            if (font_addr == 0) {
                i += byte_len;
                continue;
            }
            axk_gt20l16s_get_fonts_form_rom(font_addr >> 16 & 0xFF,
                                            font_addr >> 8 & 0xFF,
                                            font_addr & 0xFF,
                                            fontbuf, 2);
            axk_ssd1306_show_gb2312_str(x, y, (char *)fontbuf);
            x += 16;
            i += byte_len;
        } else {
            i++;
        }
    }
}


/*
 * axk_ssd1306_clear_area：在字符串末尾补齐空格
 * @brief  用于清除显示残留，实现局部擦除效果
 * @param  str：原始字符串数组
 * @param  max_len：该数据在屏幕上占据的最大字符长度
 * @return 无
 * @note   静态函数，仅本文件内可调用
 * @author Master_Yang
 * @date   2026-04-27
 */
static void axk_ssd1306_clear_area(char *str, int max_len)
{
    int len = 0;

    while (str[len] != '\0') {
        len++;
    }
    while (len < max_len) {
        str[len] = ' ';
        len++;
    }
    str[len] = '\0';
}


/*
 * axk_ssd1306_show_float：显示浮点数
 * @brief  使用放大整除法显示浮点数，自动去除尾部多余的 0
 * @param  x：起始列 (0-127)
 * @param  y：起始页 (0-7)
 * @param  font_size：字体大小枚举
 * @param  show_mode：显示模式 (0: 正常, 1: 反色)
 * @param  num：要显示的浮点数
 * @param  precision：最大保留小数位数
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_float(unsigned char x, unsigned char y,
                            AxkFontSize font_size, unsigned char show_mode,
                            float num, unsigned char precision)
{
    char buf[20];
    char final_str[24];
    long long scaled_val;
    int i, len = 0, p = 0;
    int is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    double multiplier = 1.0;
    for (i = 0; i < precision; i++) {
        multiplier *= 10.0;
    }
    scaled_val = (long long)(num * multiplier + 0.5);

    if (scaled_val == 0) {
        buf[len++] = '0';
    } else {
        while (scaled_val > 0) {
            buf[len++] = (scaled_val % 10) + '0';
            scaled_val /= 10;
        }
    }

    while (len <= precision) {
        buf[len++] = '0';
    }

    if (is_negative) {
        final_str[p++] = '-';
    }

    for (i = len - 1; i >= 0; i--) {
        if (i == precision - 1 && precision > 0) {
            final_str[p++] = '.';
        }
        final_str[p++] = buf[i];
    }
    final_str[p] = '\0';

    if (precision > 0) {
        p--;
        while (p > 0 && final_str[p] == '0') {
            final_str[p] = '\0';
            p--;
        }
        if (final_str[p] == '.') {
            final_str[p] = '\0';
        }
    }

    axk_ssd1306_clear_area(final_str, 6);
    axk_ssd1306_show_utf8_str(x, y, final_str);
}
#endif
