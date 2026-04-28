# AI 语音便携可调电源

[![License](https://img.shields.io/badge/license-GPL%203.0-blue.svg)](LICENSE)
[![MCU](https://img.shields.io/badge/MCU-STM32F103CBT6-blue)](https://www.st.com)
[![Platform](https://img.shields.io/badge/platform-STM32CubeMX%20%2B%20VS%20Code-orange)]()

> **"让电源听得懂人话，让调试解放双手"**  
> 基于 STM32 + 安信可 Ai-WV01-32S + CH224A 的 AI 语音交互便携可调电源。

---

## 📖 项目简介

随着 Type-C PD 快充协议的普及，电子工程师对便携、智能、可交互的调试电源需求日益增长。传统可调电源体积大、操作繁琐，而现有口袋电源缺乏智能化交互手段。

本项目将 **AI 语音交互** 集成到便携可调电源中，用户通过自然语音指令即可实现电压调节、状态查询、场景切换，无需手动操作按键或查看屏幕。

**硬件设计已开源（立创开源平台），本仓库为 STM32 固件源码。**

🔗 硬件工程：[oshwhub.com/ai-thiner_openteam](https://oshwhub.com/ai-thiner_openteam/ai-zhi-neng-bian-xie-ke-diao-yi-dong-dian-yuan-v1-0)

---

## 🎯 核心功能

### 🤖 AI 语音智控

| 功能 | 语音指令示例 |
|------|-------------|
| 语音设定电压 | "设置电压 20 伏"、"输出 6.6 伏" |
| 语音开关输出 | "开启输出"、"关闭电源" |
| 参数播报 | "当前功率 6.9 瓦，电流 1.3 安" |

### 👤 用户感知体验

- **OLED 屏**：0.96 寸 SPI OLED，128×64，中文显示电压/电流/功率/状态
- **按键操作**：4 个功能按键（电压 +/-、复位、输出开关）
- **喇叭 + 咪头**：语音播报 + 板载拾音

### ⚡ 供电与输出

- **输入**：单口 Type-C PD，5V–20V
- **输出**：5V–20V 可调，XT60 + 鳄鱼夹
- **协议**：PD 3.0 / PPS

---

## 📊 项目参数

| 参数 | 规格 | 备注 |
|------|------|------|
| 主控芯片 | STM32F103CBT6 | ARM Cortex-M3, 72MHz |
| AI 模组 | Ai-WV01-32S | 安信可语音交互模组 |
| 诱骗芯片 | CH224A | PD/PPS 多档位切换 |
| 电流采集 | INA226 | I2C 高精度监测 |
| 输入电压 | 5V–20V | Type-C PD |
| 输出电压 | 5V–20V | 按键 5 档 / 语音 PPS 0.1V 步进 |
| 最大功率 | 60W | 持续功率 |
| 最大电流 | 3A | 20V 档位 |
| 支持协议 | PD 3.0 / PPS | 不含 QC/AFC/FCP |
| 语音方案 | AI 在线识别 | 需 2.4G WiFi |
| PCB 尺寸 | 43.6 × 76mm | 四层板 1.6mm |

---

## 🏗️ 硬件架构

```
Type-C PD 输入
    │
    ▼
┌─────────┐    I2C     ┌──────────┐    SPI     ┌──────────┐
│ CH224A  │◄──────────►│ STM32F103│◄─────────►│ SSD1306  │
│ PD诱骗  │            │  CBT6    │           │  OLED    │
└────┬────┘            └────┬─────┘           └──────────┘
     │                      │
     │ 功率输出              │ I2C
     ▼                      ▼
  XT60+鳄鱼夹          ┌──────────┐
                       │  INA226  │
                       │ 电流采集 │
                       └──────────┘
     │                      │
     │               UART (115200)
     │                      ▼
     │              ┌──────────────┐
     │              │ Ai-WV01-32S  │
     │              │  AI 语音模组  │
     │              └──────────────┘
```

**核心模块：**
1. **CH224A** — PD 快充诱骗，I2C 控制电压档位
2. **INA226** — I2C 高精度电流/电压监测
3. **SSD1306 OLED** — SPI 驱动，GT20L16S 中文字库
4. **Ai-WV01-32S** — UART-MCP 固件，emMCP 协议通信
5. **SCT2433STER** → 5V，**ME6217C33M5G** → 3.3V 两级降压

---

## 📁 固件目录结构

```
AXK_AiPowerSupply/
├── Core/                   # CubeMX 生成的 HAL 初始化代码
├── Drivers/                # STM32F1 HAL 驱动库
├── Middlewares/            # FreeRTOS 内核
├── components/             # 项目业务模块
│   ├── app/                # 应用主任务（初始化、界面、按键处理）
│   ├── PWR/                # 电源控制（电压切换、emMCP 回调）
│   ├── ch224/              # CH224A PD 诱骗芯片驱动
│   ├── INA226/             # INA226 电流电压采集驱动
│   ├── SSD1306/            # OLED 显示驱动 + 中文字库
│   │   └── gt20l16s/       # GT20L16S 字库芯片驱动
│   ├── KEY/                # 按键扫描与消抖
│   ├── log/                # 日志模块（UART 输出）
│   └── emMCP_config.h      # emMCP 协议栈配置
├── Bsp/                    # 板级支持包
│   ├── i2c/                # 软件 I2C 驱动
│   ├── spi/                # SPI 驱动
│   └── delay/              # 微秒/毫秒延时
├── emMCP/                  # emMCP 协议栈（UART-MCP）
├── docs/                   # 项目文档
├── openocd.cfg             # OpenOCD 烧录配置
└── CMakeLists.txt          # CMake 构建配置
```

---

## 🔧 开发环境

| 工具 | 版本/说明 |
|------|----------|
| IDE | VS Code + STM32 插件 |
| 代码生成 | STM32CubeMX |
| 构建系统 | CMake + Ninja (cube-cmake) |
| 编译器 | arm-none-eabi-gcc 14.3.1 |
| 调试器 | ST-Link V2 (SWD) |
| 烧录工具 | OpenOCD 0.12.0 |
| RTOS | FreeRTOS (CMSIS-RTOS V2) |

---

## 🚀 快速开始

### 1. 克隆仓库

```bash
git clone git@github.com:MrYunYan/AI-Voice-Power.git
cd AI-Voice-Power
```

### 2. 编译

```bash
cube-cmake -S . -B build/Debug --preset Debug
cube-cmake --build build/Debug --
```

### 3. 烧录（ST-Link SWD）

```bash
openocd -f openocd.cfg -c "init; program build/Debug/AXK_AiPowerSupply.elf verify reset exit"
```

> 📌 ST-Link 仅需连接 **SWDIO / SWCLK / GND** 三线，无需 NRST。

### 4. AI 模组配网

1. 烧录 Ai-WV01-32S 的 UART-MCP 固件（`AiPi-PalChatV1_WWXH_Zh_UART-MCP_V3.3.bin`）
2. 上电后说"开始配网"，用「安信可IOT」小程序连接 2.4G WiFi
3. 访问 [xiaozhi.me](https://xiaozhi.me) 控制台，输入智能体分享码 **`A9nCXwwr6L`**，绑定设备

---

## 📜 编码规范

本项目遵循 AXK 嵌入式 C 语言编码规范，详见 [docs/编码规范.md](docs/编码规范.md)。

核心要求：
- 所有函数必须添加 `axk_` 前缀函数头注释
- 命名：函数 `axk_` snake_case，宏 `AXK_` UPPER_CASE，枚举 `Axk` CamelCase
- 缩进 4 空格，禁止 Tab

---

## 🤝 复刻说明

### 硬件复刻

PCB 源文件、BOM、原理图见立创开源平台：[工程链接](https://oshwhub.com/ai-thiner_openteam/ai-zhi-neng-bian-xie-ke-diao-yi-dong-dian-yuan-v1-0)

焊接顺序：电源管理 → MCU 及外围 → 小封装芯片 → 大件（OLED/按键/XT60）

### 注意事项

⚠️ **高电压警告**：输出最高 20V，切换前请先关闭输出  
⚠️ **网络要求**：AI 语音需 2.4G WiFi，不支持 5G 频段  
⚠️ **功率限制**：最大 60W/3A，勿超载使用  

---

## 📄 开源协议

本项目采用 **GPL 3.0** 协议。硬件设计知识产权归创作者所有，仅供学习交流，禁止商业用途。

---

## 🙏 致谢

- 安信可科技开源团队 — Ai-WV01-32S 模组及 UART-MCP 固件
- 立创开源硬件平台 — 硬件工程托管
- STMicroelectronics — STM32 HAL 库
