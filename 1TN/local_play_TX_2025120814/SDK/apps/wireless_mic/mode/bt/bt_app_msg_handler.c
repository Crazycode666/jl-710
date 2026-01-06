#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".bt_app_msg_handler.data.bss")
#pragma data_seg(".bt_app_msg_handler.data")
#pragma const_seg(".bt_app_msg_handler.text.const")
#pragma code_seg(".bt_app_msg_handler.text")
#endif
#include "le_broadcast.h"
#include "wireless_trans.h"
#include "app_le_broadcast.h"
#include "app_msg.h"
#include "le_audio_recorder.h"
#include "le_audio_player.h"
#include "bt_key_func.h"
#include "jlstream_node_cfg.h"

#include "app_config.h"
#include "jlstream.h"
#include "node_uuid.h"
#include "effects/effects_adj.h"
#include "effects/audio_llns_dns.h"
#include "effects/audio_llns.h"
#include "effects/audio_plate_reverb.h"

static bool wm_plate_reverb_bypass = 0;
static bool wm_llns_dns_bypass = 0;
static bool wm_llns_bypass = 0;
static bool wm_tx_dvol_mute = 0;
static bool wm_tx_monitor_dvol_mute = 0;
/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

extern void echo_led_on(u32 port,u32 pin,u32 gpio_port);
extern void echo_led_off(u32 port,u32 pin,u32 gpio_port);

int bt_app_msg_handler(int *msg)
{
    u8 msg_type = msg[0];

    printf("bt_app_msg type:0x%x", msg[0]);

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    if (!get_bis_connected_num()) {
        printf("LE AUDIO NOT CONNECTED, KEY EVENT FILTER");
        return 0;
    }
#endif

    switch (msg_type) {

    case APP_MSG_LE_AUDIO_ENTER_PAIR:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        app_broadcast_enter_pair(BROADCAST_ROLE_UNKNOW, 0);
#endif
        break;

    case APP_MSG_WIRELESS_MIC_1TN_ENTER_UPDATE:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        app_broadcast_enter_pair(BROADCAST_ROLE_UNKNOW, 1);
#endif
        break;

    case APP_MSG_LE_AUDIO_EXIT_PAIR:
    case APP_MSG_WIRELESS_MIC_1TN_EXIT_UPDATE:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        app_broadcast_exit_pair(BROADCAST_ROLE_UNKNOW);
#endif
        break;

    case APP_MSG_WIRELESS_MIC_TX_VOL_MUTE:
        wm_tx_dvol_mute = !wm_tx_dvol_mute;
        le_audio_wireless_mic_tx_dvol_mute(wm_tx_dvol_mute);
        break;

    case APP_MSG_WIRELESS_MIC_TX_MONIROT_VOL_MUTE:
        wm_tx_monitor_dvol_mute = !wm_tx_monitor_dvol_mute;
        le_audio_wireless_mic_tx_monitor_dvol_mute(wm_tx_monitor_dvol_mute);
        break;

    case APP_MSG_WIRELESS_MIC_TX_VOL_UP:
        le_audio_wireless_mic_tx_dvol_up();
        break;

    case APP_MSG_WIRELESS_MIC_TX_VOL_DOWN:
        le_audio_wireless_mic_tx_dvol_down();
        break;

    case APP_MSG_WIRELESS_MIC_TX_MONITOR_VOL_UP:
        le_audio_wireless_mic_tx_monitor_dvol_up();
        break;

    case APP_MSG_WIRELESS_MIC_TX_MONITOR_VOL_DOWN:
        le_audio_wireless_mic_tx_monitor_dvol_down();
        break;

#if (TCFG_KBOX_1T3_MODE_EN || WIRELESS_MIC_PRODUCT_MODE)
    case APP_MSG_WIRELESS_MIC0_VOL_UP:
        le_audio_dvol_up(0);
        y_printf("[0] Up!");
        break;
    case APP_MSG_WIRELESS_MIC0_VOL_DOWN:
        y_printf("[0] Down!");
        le_audio_dvol_down(0);
        break;
    case APP_MSG_WIRELESS_MIC1_VOL_UP:
        y_printf("[1] Up!");
        le_audio_dvol_up(1);
        break;
    case APP_MSG_WIRELESS_MIC1_VOL_DOWN:
        y_printf("[1] Down!");
        le_audio_dvol_down(1);
        break;
#endif

    case APP_MSG_PLATE_REVERB_PARAM_SWITCH:
        app_plate_reverb_parm_switch(0xff);
        break;
    case APP_MSG_PLATE_REVERB_BYPASS:
        wm_plate_reverb_bypass = (wm_plate_reverb_bypass == 0) ? 1 : 0;
        app_plate_reverb_parm_switch(wm_plate_reverb_bypass);
        if(!wm_plate_reverb_bypass)
        {
            //led_flash_del();
            //pwm_breath_led_add();
            echo_led_off(PORTC,PORT_PIN_3,PORT_OUTPUT_HIGH);
        }
        else
        {
            //pwm_breath_led_del();
            //pwm_conn_led(5000);
            //echo_led_on(PORTC,PORT_PIN_2);
             echo_led_on(PORTC,PORT_PIN_3,PORT_OUTPUT_HIGH);
        }
        break;

    case APP_MSG_LLNS_DNS_PARAM_SWITCH:
        app_llns_dns_parm_switch(0xff);
        break;
    case APP_MSG_LLNS_DNS_BYPASS:
        wm_llns_dns_bypass = (wm_llns_dns_bypass == 0) ? 1 : 0;
        app_llns_dns_parm_switch(wm_llns_dns_bypass);
        break;

    case APP_MSG_LLNS_BYPASS:
        wm_llns_bypass = (wm_llns_bypass == 0) ? 1 : 0;
        app_llns_parm_switch(wm_llns_bypass);
        break;


    case APP_MSG_LLNS_DNS_SW_LLNS:
        wm_llns_dns_bypass = (wm_llns_dns_bypass == 0) ? 1 : 0;
        wm_llns_bypass = (wm_llns_bypass == 0) ? 1 : 0;
        if (wm_llns_dns_bypass && !wm_llns_bypass) {
            app_llns_dns_parm_switch(1);
            app_llns_parm_switch(0);
        } else if (!wm_llns_dns_bypass && wm_llns_bypass) {
            app_llns_parm_switch(1);
            app_llns_dns_parm_switch(0);
        } else {
            printf("APP_MSG_LLNS_DNS_SW_LLNS error");
        }
        break;

    default:
        printf("unknow msg type:%d", msg_type);
        break;
    }

    return 0;
}

void wireless_mic_audio_status_reset(void)
{
    int ret = 0;

#if TCFG_PLATE_REVERB_NODE_ENABLE
    plate_reverb_param_tool_set reverb_cfg = {0};
    ret = jlstream_read_form_data(0, "PlateRever51011", 0, &reverb_cfg);
    if (!ret) {
        printf("read parm err, %s, %s\n", __func__, "PlateRever51011");
    } else {
        wm_plate_reverb_bypass = reverb_cfg.is_bypass;
    }
#endif


#if TCFG_LLNS_DNS_NODE_ENABLE
    llns_dns_param_tool_set llns_dns_cfg = {0};
    ret = jlstream_read_form_data(0, "LLNS_DNS1", 0, &llns_dns_cfg);
    if (!ret) {
        printf("read parm err, %s, %s\n", __func__, "LLNS_DNS1");
        wm_llns_dns_bypass = 1;
    } else {
        wm_llns_dns_bypass = llns_dns_cfg.is_bypass;
    }
#endif

#if TCFG_LLNS_NODE_ENABLE
    llns_param_tool_set llns_cfg = {0};
    ret = jlstream_read_form_data(0, "LLNS1", 0, &llns_cfg);
    if (!ret) {
        printf("read parm err, %s, %s\n", __func__, "LLNS1");
        wm_llns_bypass = 1;
    } else {
        wm_llns_bypass = llns_cfg.is_bypass;

    }
#endif
    wm_tx_dvol_mute = 0;
    wm_tx_monitor_dvol_mute = 0;
}

bool get_llns_dns_bypass_status(void)
{
    return wm_llns_dns_bypass;
}

bool get_plate_reverb_bypass_status(void)
{
    return wm_plate_reverb_bypass;
}

bool get_llns_bypass_status(void)
{
    return wm_llns_bypass;
}
