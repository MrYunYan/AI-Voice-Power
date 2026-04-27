/*
 * @file    axk_ssd1306.h
 * @brief   SSD1306 OLED 驱动头文件
 * @version V1.0
 * @date    2026-04-27
 * @author  Master_Yang
 * @license 
 * @note    
 */

#ifndef AXK_SSD1306_H
#define AXK_SSD1306_H

/* ==================================================================
 * 接口宏定义
 * ================================================================== */
#define AXK_SSD1306_SPI

#if __has_include("stm32f10x_bsp_spi.h")
#include "stm32f10x_bsp_spi.h"
#define AXK_SSD1306_ACLL(_func, ...) bsp_spi_##_func(__VA_ARGS__)
#define AXK_SSD1306_SPI_READY BSP_SPI_STATE_READY
#define AXK_SSD1306_SPI_BUSY  BSP_SPI_STATE_BUSY
#else
#define AXK_SSD1306_ACLL()
#ifdef AXK_SSD1306_SPI
#define AXK_SSD1306_SPI_BUSY
#define AXK_SSD1306_SPI_READY
#endif
#error "AXK_SSD1306_ACLL not found"
#endif

#if __has_include("axk_gt20l16s.h")
#include "axk_gt20l16s.h"
#define AXK_GT20L16_IS_AVAILABLE
#endif

/* ==================================================================
 * 命令/数据标志
 * ================================================================== */
#define AXK_SSD1306_WRITE_CMD  0
#define AXK_SSD1306_WRITE_DATA 1

/* ==================================================================
 * 类型定义
 * ================================================================== */
typedef enum {
    AXK_FONT_SIZE_8  = 0x08,
    AXK_FONT_SIZE_12 = 0x0C,
    AXK_FONT_SIZE_16 = 0x10,
    AXK_FONT_SIZE_24 = 0x18,
} AxkFontSize;

/* ==================================================================
 * 函数声明
 * ================================================================== */

/*
 * axk_ssd1306_init：初始化 SSD1306 OLED 显示屏
 * @brief  发送初始化命令序列，配置屏幕参数并清屏
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_init(void);

/*
 * axk_ssd1306_refresh：刷新 OLED 显示
 * @brief  将显存数据刷新到 OLED 屏幕
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_refresh(void);

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
void axk_ssd1306_write_byte(unsigned char data, unsigned char cmd);

/*
 * axk_ssd1306_set_color_turn：设置反色显示
 * @brief  设置 OLED 颜色翻转模式
 * @param  i：0 正常显示，1 反色显示
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_set_color_turn(unsigned char i);

/*
 * axk_ssd1306_set_display_turn：设置屏幕旋转
 * @brief  设置 OLED 扫描方向，实现 180 度旋转
 * @param  i：0 正常方向，1 旋转 180 度
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_set_display_turn(unsigned char i);

/*
 * axk_ssd1306_open_display：开启 OLED 显示
 * @brief  打开电荷泵并开启显示
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_open_display(void);

/*
 * axk_ssd1306_close_display：关闭 OLED 显示
 * @brief  关闭电荷泵并进入休眠模式
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_close_display(void);

/*
 * axk_ssd1306_clear_screen：清除屏幕内容
 * @brief  将显存全部清零并刷新显示
 * @param  无
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_clear_screen(void);

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
void axk_ssd1306_set_address(unsigned char x, unsigned char y);

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
                            unsigned char draw_mode);

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
                           unsigned char y2, unsigned char draw_mode);

/*
 * axk_ssd1306_draw_circle：绘制一个圆
 * @brief  使用中点圆算法绘制圆形
 * @param  x0：圆心列坐标 (0-127)
 * @param  y0：圆心行坐标 (0-63)
 * @param  r：半径
 * @param  draw_mode：绘制模式
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_draw_circle(unsigned char x0, unsigned char y0,
                             unsigned char r, unsigned char draw_mode);

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
                           unsigned char c);

/*
 * axk_ssd1306_show_image_128x64：显示 128x64 全屏图像
 * @brief  将 128x64 点阵图像数据刷新到屏幕
 * @param  dp：图像数据指针
 * @return 无
 * @note   无
 * @author Master_Yang
 * @date   2026-04-27
 */
void axk_ssd1306_show_image_128x64(unsigned char *dp);

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
                          unsigned char *str);

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
                             unsigned int num);

#ifdef AXK_GT20L16_IS_AVAILABLE
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
void axk_ssd1306_show_gb2312_str(unsigned char x, unsigned char y, char *str);

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
                               const char *str);

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
                            float num, unsigned char precision);
#endif

#endif /* AXK_SSD1306_H */
