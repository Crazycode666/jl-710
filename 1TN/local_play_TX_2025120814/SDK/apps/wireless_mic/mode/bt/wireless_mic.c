#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".soundbox.data.bss")
#pragma data_seg(".soundbox.data")
#pragma const_seg(".soundbox.text.const")
#pragma code_seg(".soundbox.text")
#endif
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_tone.h"
#include "soundbox.h"
#include "user_cfg.h"
#include "bt_event_func.h"
#include "dtemp_pll_trim.h"
#include "app_main.h"
#include "app_version.h"
#include "app_le_broadcast.h"
#include "app_le_connected.h"
#include "le_audio_stream.h"
#include "le_audio_player.h"
#include "le_audio_recorder.h"
#include "app_testbox.h"
#include "bt_key_func.h"
#include "clock_manager/clock_manager.h"

#define LOG_TAG             "[SOUNDBOX]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

struct bt_mode_var g_bt_hdl;

#if TCFG_AUDIO_MIC_ENABLE

static int app_bt_init();
static int app_bt_exit();

#if (LIB_DEBUG == 0)
void printf_string_buf(const uint8_t *buf, size_t len)
{
}
#endif

static u8 *bt_get_sdk_ver_info(u8 *len)
{
    const char *p = sdk_version_info_get();
    if (len) {
        *len = strlen(p);
    }

    log_info("sdk_ver:%s %x\n", p, *len);
    return (u8 *)p;
}

#ifdef CONFIG_WIRELESS_MIC_CASE_ENABLE

extern const int config_btctler_mode;
#define BT_MODE_IS(x)            (config_btctler_mode & (x))
extern const hci_transport_t *hci_transport_h4_controller_instance(void);
extern const hci_transport_t *hci_transport_uart_instance(void);
extern int btctrler_task_init(const void *transport, const void *config);
extern int btctrler_task_ready();
void (*hci_iso_receive_callback)(void *data, size_t size);
void hci_iso_receive_callback_register(void *callback)
{
    hci_iso_receive_callback = (void (*)(void *, size_t))callback;
}
extern void ll_hci_iso_free(uint8_t *packet, size_t size);
void hci_iso_handler(uint8_t *packet, size_t size)
{
    if (hci_iso_receive_callback) {
        hci_iso_receive_callback(packet, size);
    }
    ll_hci_iso_free(packet, size);
}
void hci_remove_event_handler(void *callback_handler)
{
}
int hci_packet_handler(u8 type, u8 *packet, u16 size)
{
    return 0;
}
static hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200,
    0,  // main baudrate
    0,  // flow control
    NULL,
};
int btstack_init()
{
    //printf("le_support:%x %x\n", config_btctler_modules, config_btctler_mode);
    //printf("le_config:%x %x %x %x\n", config_le_hci_connection_num, config_le_gatt_server_num, config_le_gatt_client_num, config_le_sm_support_enable);
    if (BT_MODE_IS(BT_FCC) || BT_MODE_IS(BT_FRE)) {
        btctrler_task_init((void *)hci_transport_uart_instance(), &config);
        return 0;
    }
    btctrler_task_init((void *)hci_transport_h4_controller_instance(), NULL);
    while (btctrler_task_ready() == 0) {
        os_time_dly(2);
    }
    return 1;
}
uint16_t little_endian_read_16(const uint8_t *buffer, int pos)
{
    return ((uint16_t) buffer[pos]) | (((uint16_t)buffer[(pos) + 1]) << 8);
}
uint32_t little_endian_read_24(const uint8_t *buffer, int pos)
{
    return ((uint32_t) buffer[pos]) | (((uint32_t)buffer[(pos) + 1]) << 8) | (((uint32_t)buffer[(pos) + 2]) << 16);
}
uint32_t little_endian_read_32(const uint8_t *buffer, int pos)
{
    return ((uint32_t) buffer[pos]) | (((uint32_t)buffer[(pos) + 1]) << 8) | (((uint32_t)buffer[(pos) + 2]) << 16) | (((uint32_t) buffer[(pos) + 3]) << 24);
}
//定义一些不用的空函数用于编译
u8 check_le_conn_disconnet_flag(void)
{
    return 1;
}
void set_music_vol_for_no_vol_sync_dev(u8 *addr, u8 vol)
{
}
int tws_host_set_ble_core_data(u8 *packet, int size)
{
    return 0;
}
#endif
void bt_function_select_init()
{
#if TCFG_USER_BLE_ENABLE
    u8 tmp_ble_addr[6];
    memcpy(tmp_ble_addr, (void *)bt_get_mac_addr(), 6);
    le_controller_set_mac((void *)tmp_ble_addr);
    puts("-----ble 's address-----\n");
    printf_buf((void *)bt_get_mac_addr(), 6);
#endif

#if (CONFIG_BT_MODE != BT_NORMAL)
    set_bt_enhanced_power_control(1);
#endif
}

u8 bt_app_exit_check(void)
{
    return 1;
}

bool bt_check_already_initializes(void)
{
    return g_bt_hdl.init_start;
}

struct app_mode *app_enter_bt_mode(int arg)
{
    int msg[16];
    struct bt_event *event;
    struct app_mode *next_mode;

#if TCFG_BT_RF_USE_EXT_PA_ENABLE
#if (TCFG_RF_PA_LDO_PORT != NO_CONFIG_PORT)
    gpio_set_mode(IO_PORT_SPILT(TCFG_RF_PA_LDO_PORT), PORT_OUTPUT_HIGH);
#endif
    //Note:需要在蓝牙协议栈初始化之前调用
    /* bt_set_rxtx_status_enable(1); */
    //Note:tx_io,rx_io可以只初始一个，0xffff表示不是用该io;
    extern void rf_pa_io_set(u8 tx_io, u8 rx_io, u8 en);
    rf_pa_io_set(TCFG_RF_PA_TX_PORT, TCFG_RF_PA_RX_PORT, 1);
#endif

    app_bt_init();

#ifdef CONFIG_WIRELESS_MIC_CASE_ENABLE
    bt_status_init_ok();
#endif
    mic_player_open();
    while (1) {

        if (!app_get_message(msg, ARRAY_SIZE(msg), bt_mode_key_table)) {
            continue;
        }
        next_mode = app_mode_switch_handler(msg);
        if (next_mode) {
            break;
        }

        event = (struct bt_event *)(msg + 1);

        switch (msg[0]) {
        case MSG_FROM_APP:
            bt_app_msg_handler(msg + 1);
            break;
        }

        app_default_msg_handler(msg);
    }

    app_bt_exit();

    return next_mode;
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙非后台模式退出蓝牙等待蓝牙状态可以退出
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_no_background_exit_check(void *priv)
{
    if (g_bt_hdl.init_ok == 0) {
        return;
    }

    trim_timer_del();
    g_bt_hdl.init_ok = 0;
    g_bt_hdl.init_start = 0;
    g_bt_hdl.exiting = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙非后台模式退出模式
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static u8 bt_nobackground_exit()
{
    if (!g_bt_hdl.init_start) {
        g_bt_hdl.exiting = 0;
        return 0;
    }

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    app_broadcast_uninit();
#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    app_connected_close_all(APP_CONNECTED_STATUS_STOP);
    app_connected_uninit();
#endif

    bt_no_background_exit_check(NULL);

    return 0;
}

//领夹麦形态下bypass降噪后的系统时钟。如何时钟不够请手动加大该值
#if (TCFG_DAC_NODE_ENABLE || TCFG_IIS_NODE_ENABLE || TCFG_MULTI_CH_IIS_NODE_ENABLE)
#define MIN_CLOCK_FREQ  120 * 1000000UL
#else
#define MIN_CLOCK_FREQ  96 * 1000000UL
#endif

static int app_bt_init()
{
    g_bt_hdl.init_start = 1;//蓝牙协议栈已经开始初始化标志位
    g_bt_hdl.init_ok = 0;
    g_bt_hdl.exiting = 0;
    u32 sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    clock_alloc("max", TCFG_FIX_CLOCK_FREQ);
#if (LEA_BIG_FIX_ROLE == LEA_ROLE_AS_TX)
    clock_alloc("min", MIN_CLOCK_FREQ);
#endif

    tone_player_stop();
    play_tone_file(get_tone_files()->bt_mode);

    if (g_bt_hdl.init_ok == 0) {
        bt_function_select_init();
        if (!g_bt_hdl.initializing) {
            btstack_init();
#if (TCFG_BT_BLE_TX_POWER > 4)
            //开放13dB功率
            extern u8 set_txpwr_extend_lev(u8 extend_lev);
            set_txpwr_extend_lev(1);
#endif

#if TCFG_TEST_BOX_ENABLE
            //调用该接口后，串口升级时测试盒才能获取到设备地址
            testbox_set_bt_init_ok(1);
#endif
        }
    }

    sys_auto_shut_down_enable();

    app_send_message(APP_MSG_ENTER_MODE, APP_MODE_BT);

    return 0;
}

int bt_mode_try_exit()
{
    putchar('k');

    g_bt_hdl.exiting = 1;

    bt_nobackground_exit();

    return 0;
}

static int app_bt_exit()
{
    app_send_message(APP_MSG_EXIT_MODE, APP_MODE_BT);

    sys_auto_shut_down_disable();

    return 0;
}

static int bt_mode_try_enter(int arg)
{
    return 0;
}

static const struct app_mode_ops bt_mode_ops = {
    .try_enter      = bt_mode_try_enter,
    .try_exit       = bt_mode_try_exit,
};

REGISTER_APP_MODE(bt_mode) = {
    .name   = APP_MODE_BT,
    .index  = APP_MODE_BT_INDEX,
    .ops    = &bt_mode_ops,
};

void btstack_init_for_app(void)
{
    if (g_bt_hdl.initializing) {
        return;
    }

    if (g_bt_hdl.init_ok == 0) {
        g_bt_hdl.initializing = 1;
        bt_function_select_init();
        btstack_init();
    }
}

void btstack_exit_for_app(void)
{
    if (g_bt_hdl.init_ok) {
        g_bt_hdl.init_ok = 0;
    }
}

void btstack_init_in_other_mode(void)
{
    btstack_init_for_app();
}

void btstack_exit_in_other_mode(void)
{
    btstack_exit_for_app();
}

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN)) || \
    (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
static void *wlm_tx_le_audio_open(void *args)
{
    int err;
    void *le_audio = NULL;

    struct le_audio_stream_params *params = (struct le_audio_stream_params *)args;

#if TCFG_LLNS_DNS_NODE_ENABLE
    if (get_llns_dns_bypass_status()) {
        params->latency =  get_big_tx_latency_base() + 2000;
    }
#endif

    r_printf("tx_latency:%d", params->latency);

    le_audio = le_audio_stream_create(params->conn, &params->fmt);

#if TCFG_IIS_RX_NODE_ENABLE
    err = le_audio_iis_recorder_open((void *)&params->fmt, le_audio, params->latency);
#else
    err = le_audio_mic_recorder_open((void *)&params->fmt, le_audio, params->latency);
#endif
    if (err != 0) {
        ASSERT(0, "recorder open fail");
    }
#if LEA_LOCAL_SYNC_PLAY_EN
    err = le_audio_player_open(le_audio, params);
    if (err != 0) {
        ASSERT(0, "player open fail");
    }
#endif

    return le_audio;
}

static int wlm_tx_le_audio_close(void *le_audio)
{
    if (!le_audio) {
        return -EPERM;
    }

    //关闭广播音频播放
#if LEA_LOCAL_SYNC_PLAY_EN
    le_audio_player_close(le_audio);
#endif

#if TCFG_IIS_RX_NODE_ENABLE
    le_audio_iis_recorder_close();
#else
    le_audio_mic_recorder_close();
#endif
    le_audio_stream_free(le_audio);

    return 0;
}

static int wlm_rx_le_audio_open(void *rx_audio, void *args)
{
    int err;
    struct le_audio_player_hdl *rx_audio_hdl = (struct le_audio_player_hdl *)rx_audio;

    if (!rx_audio) {
        return -EPERM;
    }

    //打开广播音频播放
    struct le_audio_stream_params *params = (struct le_audio_stream_params *)args;
    rx_audio_hdl->le_audio = le_audio_stream_create(params->conn, &params->fmt);
    rx_audio_hdl->rx_stream = le_audio_stream_rx_open(rx_audio_hdl->le_audio, params->fmt.coding_type);
    err = le_audio_player_open(rx_audio_hdl->le_audio, params);
    if (err != 0) {
        ASSERT(0, "player open fail");
    }

    return 0;
}

static int wlm_rx_le_audio_close(void *rx_audio)
{
    struct le_audio_player_hdl *rx_audio_hdl = (struct le_audio_player_hdl *)rx_audio;

    if (!rx_audio) {
        return -EPERM;
    }

    //关闭广播音频播放
    le_audio_player_close(rx_audio_hdl->le_audio);
    le_audio_stream_rx_close(rx_audio_hdl->rx_stream);
    le_audio_stream_free(rx_audio_hdl->le_audio);

    return 0;
}
void mic_player_close();
int mic_player_open(void);

const struct le_audio_mode_ops le_audio_wireless_mic_ops = {
    .local_audio_open = mic_player_open,
    .local_audio_close = mic_player_close,
    .tx_le_audio_open = wlm_tx_le_audio_open,
    .tx_le_audio_close = wlm_tx_le_audio_close,
    .rx_le_audio_open = wlm_rx_le_audio_open,
    .rx_le_audio_close = wlm_rx_le_audio_close,
};
#endif
#endif

