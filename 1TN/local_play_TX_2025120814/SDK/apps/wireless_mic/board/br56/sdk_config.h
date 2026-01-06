/**
* 注意点：
* 0.此文件变化，在工具端会自动同步修改到工具配置中
* 1.功能块通过【---------xxx------】和 【#endif // xxx 】，是工具识别的关键位置，请勿随意改动
* 2.目前工具暂不支持非文件已有的C语言语法，此文件应使用文件内已有的语法增加业务所需的代码，避免产生不必要的bug
* 3.修改该文件出现工具同步异常或者导出异常时，请先检查文件内语法是否正常
**/

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "jlstream_node_cfg.h"

// ------------JLStudio Auto generate------------
#define PROJECT_CONFIG_NAME "" // 默认配置为空，非默认配置为配置名称
// ------------JLStudio Auto generate------------

// ------------板级配置.json------------
#define TCFG_DEBUG_UART_ENABLE 0 // 调试串口
#if TCFG_DEBUG_UART_ENABLE
#define TCFG_DEBUG_UART_TX_PIN IO_PORT_DP // 输出IO
#define TCFG_DEBUG_UART_BAUDRATE 2000000 // 波特率
#define TCFG_EXCEPTION_LOG_ENABLE 1 // 打印异常信息
#define TCFG_EXCEPTION_RESET_ENABLE 1 // 异常自动复位
#endif // TCFG_DEBUG_UART_ENABLE

#define TCFG_CFG_TOOL_ENABLE 0 // FW编辑、在线调音
#if TCFG_CFG_TOOL_ENABLE
#define TCFG_ONLINE_TX_PORT IO_PORT_DP // 串口引脚TX
#define TCFG_ONLINE_RX_PORT IO_PORT_DM // 串口引脚RX
#define TCFG_COMM_TYPE TCFG_USB_COMM // 通信方式
#endif // TCFG_CFG_TOOL_ENABLE

#define CONFIG_SPI_DATA_WIDTH 4 // flash通信
#define CONFIG_SPI_MODE 0 // flash模式
#define CONFIG_FLASH_SIZE 1048576 // flash容量
#define TCFG_VM_SIZE 32 // VM大小（K）

#define TCFG_PWMLED_ENABLE 0 // LED配置
#if TCFG_PWMLED_ENABLE
#define TCFG_LED_LAYOUT ONE_IO_ONE_LED // 连接方式
#define TCFG_LED_RED_ENABLE 1 // 红灯(Red)
#define TCFG_LED_RED_GPIO IO_PORT_DP // IO
#define TCFG_LED_RED_LOGIC BRIGHT_BY_LOW // 点亮方式
#define TCFG_LED_BLUE_ENABLE 1 // 蓝灯(Blue)
#define TCFG_LED_BLUE_GPIO IO_PORT_DM // IO
#define TCFG_LED_BLUE_LOGIC BRIGHT_BY_HIGH // 点亮方式
#endif // TCFG_PWMLED_ENABLE

#define FUSB_MODE 1 // USB工作模式
#define USB_MALLOC_ENABLE 1 // 从机使用malloc
#define TCFG_USB_SLAVE_MSD_ENABLE 0 // 读卡器使能
#define TCFG_USB_SLAVE_HID_ENABLE 0 // HID使能
#define MSD_BLOCK_NUM 1 // MSD缓存块数
#define USB_AUDIO_VERSION USB_AUDIO_VERSION_1_0 // UAC协议版本
#define TCFG_USB_SLAVE_AUDIO_SPK_ENABLE 0 // USB扬声器使能
#define SPK_AUDIO_RATE_NUM 1 // SPK采样率列表
#define SPK_AUDIO_RES 16 // SPK位宽1
#define SPK_AUDIO_RES_2 0 // SPK位宽2
#define TCFG_USB_SLAVE_AUDIO_MIC_ENABLE 0 // USB麦克风使能
#define MIC_AUDIO_RATE_NUM 1 // MIC采样率列表
#define MIC_AUDIO_RES 16 // MIC位宽1
#define MIC_AUDIO_RES_2 0 // MIC位宽2
#define MIC_CHANNEL 1 // MIC声道数
#define TCFG_SW_I2C0_CLK_PORT NO_CONFIG_PORT // 软件iic CLK脚
#define TCFG_SW_I2C0_DAT_PORT NO_CONFIG_PORT // 软件iic DATA脚
#define TCFG_SW_I2C0_DELAY_CNT 50 // iic 延时
#define TCFG_HW_I2C0_CLK_PORT NO_CONFIG_PORT // 硬件iic CLK脚
#define TCFG_HW_I2C0_DAT_PORT NO_CONFIG_PORT // 硬件iic DATA脚
#define TCFG_HW_I2C0_CLK 100000 // 硬件iic波特率

#define TCFG_HW_SPI1_ENABLE 0 // 硬件SPI1
#if TCFG_HW_SPI1_ENABLE
#define TCFG_HW_SPI1_PORT_CLK NO_CONFIG_PORT // SPI1 CLK脚
#define TCFG_HW_SPI1_PORT_DO NO_CONFIG_PORT // SPI1  DO脚
#define TCFG_HW_SPI1_PORT_DI NO_CONFIG_PORT // SPI1  DI脚
#define TCFG_HW_SPI1_MODE SPI_MODE_BIDIR_1BIT // SPI1 模式
#define TCFG_HW_SPI1_ROLE SPI_ROLE_MASTER // SPI1  ROLE
#define TCFG_HW_SPI1_BAUD 24000000 // SPI1  时钟
#endif // TCFG_HW_SPI1_ENABLE

#define TCFG_HW_SPI2_ENABLE 0 // 硬件SPI2
#if TCFG_HW_SPI2_ENABLE
#define TCFG_HW_SPI2_PORT_CLK NO_CONFIG_PORT // SPI2 CLK脚
#define TCFG_HW_SPI2_PORT_DO NO_CONFIG_PORT // SPI2  DO脚
#define TCFG_HW_SPI2_PORT_DI NO_CONFIG_PORT // SPI2  DI脚
#define TCFG_HW_SPI2_MODE SPI_MODE_BIDIR_1BIT // SPI2 模式
#define TCFG_HW_SPI2_ROLE SPI_ROLE_MASTER // SPI2  ROLE
#define TCFG_HW_SPI2_BAUD 24000000 // SPI2  时钟
#endif // TCFG_HW_SPI2_ENABLE

#define TCFG_IO_CFG_AT_POWER_ON 0 // 开机时IO配置

#define TCFG_IO_CFG_AT_POWER_OFF 0 // 关机时IO配置

#define TCFG_CHARGESTORE_PORT IO_PORTP_00 // 通信IO
#define TDM_WORK_MODE 4 // Work Mode
#define TDM_CH_NUM 2 // Channels
#define TCFG_FREE_ICACHE0_WAY_NUM 0 // icache0作为普通ram使用个数
// ------------板级配置.json------------

// ------------功能配置.json------------
#define TCFG_AUDIO_MIC_ENABLE 1 // 无线mic模式
#define TCFG_FIX_CLOCK_FREQ 240000000 // 固定时钟频率

#define TCFG_USER_UART_DEMO_EN 0 // 串口通讯
#if TCFG_USER_UART_DEMO_EN
#define UART_DEMO_TX_PIN IO_PORTC_01 // 发送引脚
#define UART_DEMO_RX_PIN IO_PORTC_01 // 接收引脚
#define USER_UART_DEMO_BAUD 115200 // 波特率
#define UART_DEMO_RX_BUF_SIZE 32 // 传输数据长度
#endif // TCFG_USER_UART_DEMO_EN
// ------------功能配置.json------------

// ------------按键配置.json------------
#define TCFG_LONG_PRESS_RESET_ENABLE 0 // 非按键长按复位配置
#if TCFG_LONG_PRESS_RESET_ENABLE
#define TCFG_LONG_PRESS_RESET_PORT IO_PORTB_01 // 复位IO
#define TCFG_LONG_PRESS_RESET_TIME 8 // 复位时间
#define TCFG_LONG_PRESS_RESET_LEVEL 0 // 复位电平
#define TCFG_LONG_PRESS_RESET_INSIDE_PULL_UP_DOWN 0 // 使用IO内置上下拉
#endif // TCFG_LONG_PRESS_RESET_ENABLE

#define TCFG_IOKEY_ENABLE 0 // IO按键配置

#define TCFG_ADKEY_ENABLE 1 // AD按键配置

#define TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE 0 // 内置触摸在线调试

#define TCFG_LP_TOUCH_KEY_ENABLE 0 // 内置触摸按键配置、内置触摸公共配置
#if TCFG_LP_TOUCH_KEY_ENABLE
#define TCFG_LP_KEY_LIMIT_VOLTAGE_DELTA 800 // 上下限电压差
#define TCFG_LP_KEY_CHARGE_FREQ_KHz 2500 // 充放电频率
#define TCFG_LP_KEY_ENABLE_IN_CHARGE 0 // 充电保持触摸
#define TCFG_LP_KEY_LONG_PRESS_RESET 1 // 长按复位功能
#define TCFG_LP_KEY_LONG_PRESS_RESET_TIME 8000 // 长按复位时间
#define TCFG_LP_KEY_SLIDE_ENABLE 0 // 两个按键滑动
#define TCFG_LP_KEY_SLIDE_VALUE KEY_SLIDER // 键值
#define TCFG_LP_EARTCH_KEY_ENABLE 0 // 入耳检测总开关
#define TCFG_LP_EARTCH_DETECT_RELY_AUDIO 0 // 检测方式
#endif // TCFG_LP_TOUCH_KEY_ENABLE


// ------------按键配置.json------------

// ------------电源配置.json------------
#define TCFG_CLOCK_OSC_HZ 24000000 // 晶振频率
#define TCFG_LOWPOWER_OSC_TYPE OSC_TYPE_LRC // 低功耗时钟源
#define TCFG_LOWPOWER_POWER_SEL PWR_DCDC15 // 电源模式
#define TCFG_POWER_SUPPLY_MODE 2 // 供电模式选择
#define TCFG_LOWPOWER_VDDIOM_LEVEL VDDIOM_VOL_34V // IOVDD
#define TCFG_LOWPOWER_VDDIOW_LEVEL VDDIOM_VOL_26V // 弱IOVDD
#define CONFIG_LVD_LEVEL 2.7v // LVD档位
#define TCFG_MAX_LIMIT_SYS_CLOCK 0 // 上限时钟设置
#define TCFG_DCDC_L 1 // DCDC电感
#define TCFG_LOWPOWER_LOWPOWER_SEL 0 // 低功耗模式
#define TCFG_AUTO_POWERON_ENABLE 1 // 上电自动开机

#define TCFG_SYS_LVD_EN 1 // 电池电量检测
#if TCFG_SYS_LVD_EN
#define TCFG_POWER_OFF_VOLTAGE 3200 // 关机电压(mV)
#define TCFG_POWER_WARN_VOLTAGE 3300 // 低电电压(mV)
#endif // TCFG_SYS_LVD_EN

#define TCFG_CHARGESTORE_ENABLE 0 // 智能仓

#define TCFG_CHARGE_ENABLE 1 // 充电配置
#if TCFG_CHARGE_ENABLE
#define TCFG_CHARGE_TRICKLE_MA CHARGE_mA_60 // 涓流档位选择
#define TCFG_CHARGE_TRICKLE_DIV CHARGE_DIV_3 // 涓流分频系数
#define TCFG_CHARGE_MA CHARGE_mA_180 // 恒流档位选择
#define TCFG_CHARGE_DIV CHARGE_DIV_1 // 恒流分频系数
#define TCFG_CHARGE_FULL_V CHARGE_FULL_V_MIN_4200 // 截止电压
#define TCFG_CHARGE_FULL_MA CHARGE_FC_IS_CC_DIV_5 // 截止电流
#define TCFG_CHARGE_POWERON_ENABLE 0 // 开机充电
#define TCFG_CHARGE_OFF_POWERON_EN 1 // 拔出开机
#define TCFG_CHARGE_NVDC_EN 1 // NVDC架构使能
#define TCFG_RECHARGE_ENABLE 0 // 复充使能
#define TCFG_RECHARGE_VOLTAGE 4000 // 复充电压(mV）
#define TCFG_LDOIN_PULLDOWN_EN 1 // 下拉电阻开关
#define TCFG_LDOIN_PULLDOWN_LEV 3 // 下拉电阻档位
#define TCFG_LDOIN_PULLDOWN_KEEP 0 // 下拉电阻保持开关
#define TCFG_LDOIN_ON_FILTER_TIME 100 // 入舱滤波时间(ms)
#define TCFG_LDOIN_OFF_FILTER_TIME 200 // 出舱滤波时间(ms)
#define TCFG_LDOIN_KEEP_FILTER_TIME 440 // 维持电压滤波时间(ms)
#endif // TCFG_CHARGE_ENABLE

#define TCFG_BATTERY_CURVE_ENABLE 1 // 电池曲线配置

// ------------电源配置.json------------

// ------------UI配置.json------------
#define TCFG_UI_ENABLE 0 // UI配置
#if TCFG_UI_ENABLE
#define CONFIG_UI_STYLE STYLE_JL_LED7 // UI类型
#define TCFG_LED7_RUN_RAM 1 // LED屏驱动跑RAM
#define TCFG_UI_LED7_ENABLE 1 // LED7脚数码管屏
#define TCFG_LED7_PIN0 NO_CONFIG_PORT // LED引脚0
#define TCFG_LED7_PIN1 NO_CONFIG_PORT // LED引脚1
#define TCFG_LED7_PIN2 NO_CONFIG_PORT // LED引脚2
#define TCFG_LED7_PIN3 NO_CONFIG_PORT // LED引脚3
#define TCFG_LED7_PIN4 NO_CONFIG_PORT // LED引脚4
#define TCFG_LED7_PIN5 NO_CONFIG_PORT // LED引脚5
#define TCFG_LED7_PIN6 NO_CONFIG_PORT // LED引脚6
#endif // TCFG_UI_ENABLE
// ------------UI配置.json------------

// ------------蓝牙配置.json------------
#define CONFIG_BT_MODE 1 // 模式选择
#define TCFG_NORMAL_SET_DUT_MODE 0 // DUT使能

#define TCFG_USER_BLE_ENABLE 1 // BLE
#if TCFG_USER_BLE_ENABLE
#define TCFG_BT_BLE_TX_POWER 0 // 最大发射功率
#endif // TCFG_USER_BLE_ENABLE

#define TCFG_BT_RF_USE_EXT_PA_ENABLE 0 // 蓝牙PA配置
#if TCFG_BT_RF_USE_EXT_PA_ENABLE
#define TCFG_RF_PA_LDO_PORT NO_CONFIG_PORT // PA供电引脚
#define TCFG_RF_PA_TX_PORT NO_CONFIG_PORT // TX_EN引脚
#define TCFG_RF_PA_RX_PORT NO_CONFIG_PORT // RX_EN引脚
#endif // TCFG_BT_RF_USE_EXT_PA_ENABLE

#define TCFG_AUTO_SHUT_DOWN_TIME 0 // 无连接关机时间(s)
#define TCFG_RF_OOB_SEL 0 // oob
// ------------蓝牙配置.json------------

// ------------公共配置.json------------
#define TCFG_LE_AUDIO_APP_CONFIG LE_AUDIO_JL_BIS_TX_EN // le_audio 应用选择
#define LEA_BIG_FIX_ROLE LEA_ROLE_AS_TX // JL_BIS角色
#define WIRELESS_MIC_PRODUCT_MODE ADAPTER_2T1R_MODE // 产品形态
#define LEA_TRANS_MODE LEA_TRANS_SIMPLEX // 音频传输方式
#define LE_AUDIO_CODEC_TYPE 0xa000000 // 编解码格式
#define LE_AUDIO_CODEC_CHANNEL 1 // 编解码声道数
#define LE_AUDIO_CODEC_FRAME_LEN 50 // 帧持续时间
#define LE_AUDIO_CODEC_SAMPLERATE 32000 // 采样率
#define LEA_TX_DEC_OUTPUT_CHANNEL 37 // 发送端解码输出
#define LEA_RX_DEC_OUTPUT_CHANNEL 37 // 接收端解码输出
#define TCFG_JLA_V2_USED_FRAME_LEN_MASK (1 << 9) // JLA_V2点数

#define TCFG_BB_PKT_V3_EN 1 // BB_PKT_V3功能
#if TCFG_BB_PKT_V3_EN
#define TCFG_BB_PKT_V3_LEVEL_SEL 1 // 挡位选择
#endif // TCFG_BB_PKT_V3_EN

#define TCFG_JL_BIS_PWR_CTRL_EN 1 // PWR_CTRL功能

// ------------公共配置.json------------

// ------------JL_BIS配置.json------------
#define WIRELESS_MIC_BST_BIND_EN 0 // 绑定使能
#define LEA_BIG_CUSTOM_DATA_EN 0 // 1TN自定义数据传输

#define TCFG_JL_BIS_DEV_FILTER 0 // 设备过滤配置
#if TCFG_JL_BIS_DEV_FILTER
#define TCFG_JL_BIS_SCAN_RSSI -50 // 信号强度过滤
#endif // TCFG_JL_BIS_DEV_FILTER

// ------------JL_BIS配置.json------------

// ------------升级配置.json------------
#define TCFG_UPDATE_ENABLE 1 // 升级选择
#if TCFG_UPDATE_ENABLE
#define TCFG_UPDATE_STORAGE_DEV_EN 0 // 设备升级
#define TCFG_UPDATE_BLE_TEST_EN 1 // ble蓝牙升级
#define TCFG_TEST_BOX_ENABLE 0 // 测试盒串口升级
#define CONFIG_UPDATE_JUMP_TO_MASK 0 // 升级维持io使能
#define CONFIG_SD_LATCH_IO 0 // 升级需要维持的IO
#endif // TCFG_UPDATE_ENABLE
// ------------升级配置.json------------

// ------------音频配置.json------------
#define TCFG_AUDIO_DAC_CONNECT_MODE DAC_OUTPUT_MONO_R // 声道配置
#define TCFG_AUDIO_DAC_MODE DAC_MODE_DIFF // 输出方式
#define TCFG_AUDIO_DAC_LIGHT_CLOSE_ENABLE 0X0 // 轻量关闭
#define TCFG_AUDIO_DAC_BUFFER_TIME_MS 50 // 缓冲长度(ms)
#define TCFG_DAC_PERFORMANCE_MODE DAC_MODE_HIGH_PERFORMANCE // 性能模式
#define TCFG_AUDIO_DAC_PA_ISEL0 5 // PA_ISEL0
#define TCFG_AUDIO_DAC_PA_ISEL1 6 // PA_ISEL1
#define TCFG_AUDIO_VCM_CAP_EN 0x1 // VCM电容
#define TCFG_DAC_POWER_MODE 1 // 输出功率
#define TCFG_AUDIO_L_CHANNEL_GAIN 0x03 // L Channel
#define TCFG_AUDIO_R_CHANNEL_GAIN 0x03 // R Channel
#define TCFG_AUDIO_DIGITAL_GAIN 0 // Digital Gain

#define TCFG_AUDIO_ADC_ENABLE 1 // ADC配置
#if TCFG_AUDIO_ADC_ENABLE
#define TCFG_AUDIO_MIC_LDO_VSEL 5 // MICLDO电压
#define TCFG_ADC_PERFORMANCE_MODE ADC_MODE_HIGH_PERFORMANCE // 性能模式
#define TCFG_ADC_DIGITAL_GAIN 0 // 数字增益
#define TCFG_ADC0_ENABLE 1 // 使能
#define TCFG_ADC0_MODE 1 // 模式
#define TCFG_ADC0_AIN_SEL 1 // 输入端口
#define TCFG_ADC0_BIAS_SEL 1 // 供电端口
#define TCFG_ADC0_BIAS_RSEL 4 // MIC BIAS上拉电阻挡位
#define TCFG_ADC0_POWER_IO 0 // IO供电选择
#define TCFG_ADC0_DCC_EN 1 // DCC使能
#define TCFG_ADC0_DCC_LEVEL 1 // DCC 截止频率
#define TCFG_ADC1_ENABLE 0 // 使能
#define TCFG_ADC1_MODE 1 // 模式
#define TCFG_ADC1_AIN_SEL 1 // 输入端口
#define TCFG_ADC1_BIAS_SEL 1 // 供电端口
#define TCFG_ADC1_BIAS_RSEL 4 // MIC BIAS上拉电阻挡位
#define TCFG_ADC1_POWER_IO 0 // IO供电选择
#define TCFG_ADC1_DCC_EN 1 // DCC使能
#define TCFG_ADC1_DCC_LEVEL 1 // DCC 截止频率
#endif // TCFG_AUDIO_ADC_ENABLE

#define TCFG_AUDIO_GLOBAL_SAMPLE_RATE 48000 // 全局采样率
#define TCFG_ADC_IRQ_INTERVAL 5000 // ADC中断间隔(us)
#define TCFG_AUDIO_ADC_SAMPLE_RATE 32000 // ADC固定采样率
#define TCFG_AUDIO_DAC_POWER_ON_AT_SETUP 1 // 开机时打开DAC
#define TCFG_DATA_EXPORT_UART_TX_PORT IO_PORT_DM // 串口发送引脚
#define TCFG_DATA_EXPORT_UART_BAUDRATE 2000000 // 串口波特率
// ------------音频配置.json------------
#endif
