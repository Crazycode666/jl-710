#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".poweroff.data.bss")
#pragma data_seg(".poweroff.data")
#pragma const_seg(".poweroff.text.const")
#pragma code_seg(".poweroff.text")
#endif

#include "app_config.h"
#include "app_tone.h"
#include "app_main.h"
#include "soundbox.h"
#include "idle.h"
#include "app_charge.h"
#include "poweroff.h"
#include "app_le_connected.h"
#include "app_le_broadcast.h"
#include "le_broadcast.h"

#define LOG_TAG             "[POWEROFF]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

static u16 g_poweroff_timer = 0;
static u16 g_bt_detach_timer = 0;

static void sys_auto_shut_down_deal(void *priv);


void sys_auto_shut_down_disable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    log_info("sys_auto_shut_down_disable\n");
    if (g_poweroff_timer) {
        sys_timeout_del(g_poweroff_timer);
        g_poweroff_timer = 0;
    }
#endif
}

void sys_auto_shut_down_enable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    //bis发射或者正在监听时不能自动关机
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    if ((get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) || (get_receiver_connected_status())) {
        log_error("sys_auto_shut_down_enable cannot in le audio open\n");
        return;
    }
#endif

    //cis连接时不能自动关机
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    if (app_get_connected_role() && (!(app_get_connected_role() & BIT(7)))) {
        log_error("sys_auto_shut_down_enable cannot in le audio open\n");
        return;
    }
#endif

    log_info("sys_auto_shut_down_enable\n");

    if (g_poweroff_timer == 0) {
        g_poweroff_timer = sys_timeout_add(NULL, sys_auto_shut_down_deal,
                                           app_var.auto_off_time * 1000);
    }
#endif
}

static void sys_auto_shut_down_deal(void *priv)
{
    sys_enter_soft_poweroff(POWEROFF_NORMAL);
}

static void wait_exit_btstack_flag(void *_reason)
{
    int reason = (int)_reason;

    lmp_hci_reset();
    os_time_dly(2);
    sys_timer_del(g_bt_detach_timer);

    switch (reason) {
    case POWEROFF_NORMAL:
        log_info("task_switch to idle...\n");
        app_send_message2(APP_MSG_GOTO_MODE, APP_MODE_IDLE, IDLE_MODE_PLAY_POWEROFF);
        break;
    case POWEROFF_RESET:
        log_info("cpu_reset!!!\n");
        cpu_reset();
        break;
    case POWEROFF_POWER_KEEP:
#if TCFG_CHARGE_ENABLE
        app_charge_power_off_keep_mode();
#endif
        break;
    }
}


void sys_enter_soft_poweroff(enum poweroff_reason reason)
{
    log_info("sys_enter_soft_poweroff: %d\n", reason);

    if (app_var.goto_poweroff_flag) {
        return;
    }

    app_var.goto_poweroff_flag = 1;
    app_var.goto_poweroff_cnt = 0;
    sys_auto_shut_down_disable();

    app_send_message(APP_MSG_POWER_OFF, 0);

#if (SYS_DEFAULT_VOL == 0)
    syscfg_write(CFG_SYS_VOL, &app_var.music_volume, 2);
#endif

    g_bt_detach_timer = sys_timer_add((void *)reason, wait_exit_btstack_flag, 50);
}
