#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".app_default_msg_handler.data.bss")
#pragma data_seg(".app_default_msg_handler.data")
#pragma const_seg(".app_default_msg_handler.text.const")
#pragma code_seg(".app_default_msg_handler.text")
#endif
#include "app_main.h"
#include "app_config.h"
#include "app_default_msg_handler.h"
#include "dev_status.h"
#include "audio_config.h"
#include "app_tone.h"
#include "media/automute.h"
#include "idle.h"
#include "soundbox.h"
#include "asm/charge.h"
#include "le_broadcast.h"
#include "app_le_broadcast.h"
#include "app_le_connected.h"
#include "le_audio_player.h"
#include "usb/device/usb_stack.h"

static u8 sys_audio_mute_statu = 0;//记录 audio dac mute
u8 get_sys_aduio_mute_statu(void)
{
    return 	app_audio_get_dac_digital_mute();
}

void app_common_key_msg_handler(int *msg)
{
    switch (msg[0]) {
    case APP_MSG_VOL_UP:
        app_audio_volume_up(1);

        if (app_audio_get_volume(APP_AUDIO_CURRENT_STATE) == app_audio_get_max_volume()) {
            if (tone_player_runing() == 0) {
#if TCFG_MAX_VOL_PROMPT
                play_tone_file(get_tone_files()->max_vol);
#endif
            }
        }
        app_send_message(APP_MSG_VOL_CHANGED, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
        break;

    case APP_MSG_VOL_DOWN:
        app_audio_volume_down(1);
        app_send_message(APP_MSG_VOL_CHANGED, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
        break;

    case APP_MSG_SYS_MUTE:
        sys_audio_mute_statu = app_audio_get_dac_digital_mute() ^ 1;
        if (sys_audio_mute_statu) {
            app_audio_mute(AUDIO_MUTE_DEFAULT);
        } else {
            app_audio_mute(AUDIO_UNMUTE_DEFAULT);
        }
        app_send_message(APP_MSG_MUTE_CHANGED, sys_audio_mute_statu);
        break;

    case APP_MSG_LE_BROADCAST_SW:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        g_printf("APP_MSG_LE_BROADCAST_SW");
        app_broadcast_switch();
#endif
        break;

    case APP_MSG_LE_CONNECTED_SW:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
        app_connected_switch();
#endif
        break;

    case APP_MSG_LE_AUDIO_ENTER_PAIR:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        app_broadcast_enter_pair(BROADCAST_ROLE_UNKNOW, 0);
#endif
        break;

    case APP_MSG_LE_AUDIO_EXIT_PAIR:
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
        app_broadcast_exit_pair(BROADCAST_ROLE_UNKNOW);
#endif
        break;

    }
}

void app_common_device_event_handler(int *msg)
{
    int ret = 0;
    const char *usb_msg = NULL;
    u8 app  = 0xff ;
    switch (msg[0]) {
    case DEVICE_EVENT_FROM_OTG:
        usb_msg = (const char *)msg[2];
        if (usb_msg[0] == 's') {
#if TCFG_USB_SLAVE_ENABLE
            ret = pc_device_event_handler(msg);
            if (ret == 1) {
                if (true != app_in_mode(APP_MODE_PC)) {
                    app = APP_MODE_PC;
                }
            } else if (ret == 2) {
                app_send_message(APP_MSG_GOTO_NEXT_MODE, 0);
            }
#endif
            break;
        } else if (usb_msg[0] == 'h') {
            ///是主机, 统一于SD卡等响应主机处理，这里不break
        } else {
            break;
        }

#if (TCFG_SD0_ENABLE || TCFG_USB_HOST_ENABLE)
    case DRIVER_EVENT_FROM_SD0:
    case DRIVER_EVENT_FROM_SD1:
    case DRIVER_EVENT_FROM_SD2:
    case DEVICE_EVENT_FROM_USB_HOST:
        ret = dev_status_event_filter(msg);///解码设备上下线， 设备挂载等处理
        if (ret == true) {
            if (msg[1] == DEVICE_EVENT_IN) {
            }
        }
        break;
#endif

    default:
        /* printf("unknow SYS_DEVICE_EVENT!!, %x\n", (u32)event->arg); */
        break;
    }

    if (app != 0xff) {
#if (TCFG_CHARGE_ENABLE && (!TCFG_CHARGE_POWERON_ENABLE))
        if (get_charge_online_flag() && app != APP_MODE_PC) {
            return;
        }
#endif
        app_send_message2(APP_MSG_GOTO_MODE, app, 0);
    }
}

static void app_common_app_event_handler(int *msg)
{
    switch (msg[0]) {
    case APP_MSG_KEY_POWER_OFF:
    case APP_MSG_KEY_POWER_OFF_HOLD:
        if (msg[1] == APP_KEY_MSG_FROM_TWS) { //来自tws的按键关机消息不响应
            break;
        }
        power_off_deal(APP_MSG_KEY_POWER_OFF);
        break;
    case APP_MSG_KEY_POWER_OFF_RELEASE:
        goto_poweroff_first_flag = 0;
        break;
    case APP_MSG_KEY_POWER_OFF_INSTANTLY:
        power_off_instantly();
        break;
    default:
        break;
    }
}

void app_default_msg_handler(int *msg)
{
    const struct app_msg_handler *handler;
    struct app_mode *mode;

    mode = app_get_current_mode();
    //消息继续分发
#if (TCFG_BT_BACKGROUND_ENABLE)
    if (!bt_background_msg_forward_filter(msg))
#endif
    {
        for_each_app_msg_handler(handler) {
            if (handler->from != msg[0]) {
                continue;
            }
            if (mode && mode->name == handler->owner) {
                continue;
            }

            /*蓝牙后台情况下，消息仅转发给后台处理*/
            handler->handler(msg + 1);
        }
    }

    switch (msg[0]) {
    case MSG_FROM_DEVICE:
        app_common_device_event_handler(msg + 1);
        break;
    case MSG_FROM_APP:
        app_common_app_event_handler(msg + 1);
    default:
        break;
    }
}
