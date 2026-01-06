#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".idle.data.bss")
#pragma data_seg(".idle.data")
#pragma const_seg(".idle.text.const")
#pragma code_seg(".idle.text")
#endif
#include "idle.h"
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_tone.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "user_cfg.h"
#include "audio_config.h"
#include "dev_manager.h"

#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

static int app_idle_init(int param);
void app_idle_exit();

#define POWER_ON_CNT         10
#define POWER_OFF_CNT       10

static u8 goto_poweron_cnt = 0;
static u8 goto_poweron_flag = 0;
static u8 goto_poweroff_cnt = 0;
unsigned char goto_poweroff_first_flag = 0;
static u8 goto_poweroff_flag = 0;
static u8 power_off_tone_play_flag = 0;
static u16 wait_device_online_timer = 0;
static u8 idle_enter_param = 0;

void power_off_wait_ui()
{
}

void idle_key_poweron_deal(int msg)
{
    if (idle_enter_param == IDLE_MODE_WAIT_DEVONLINE) { //如果是等待设备上线过程不响应按键消息
        return;
    }
    switch (msg) {
    case APP_MSG_KEY_POWER_ON:
        goto_poweron_cnt = 0;
        goto_poweron_flag = 1;
        break;
    case APP_MSG_KEY_POWER_ON_HOLD:
        printf("poweron flag:%d cnt:%d\n", goto_poweron_flag, goto_poweron_cnt);
        if (goto_poweron_flag) {
            goto_poweron_cnt++;
            if (goto_poweron_cnt >= POWER_ON_CNT) {
                goto_poweron_cnt = 0;
                goto_poweron_flag = 0;
                app_var.goto_poweroff_flag = 0;
                app_var.play_poweron_tone = 0;
                app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);
            }
        }
        break;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   poweroff 长按等待 关闭蓝牙
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void power_off_deal(int msg)
{
    switch (msg) {
    case APP_MSG_KEY_POWER_OFF:
    case APP_MSG_KEY_POWER_OFF_HOLD:
        if (goto_poweroff_first_flag == 0) {
            goto_poweroff_first_flag = 1;
            goto_poweroff_cnt = 0;
            goto_poweroff_flag = 1;
            break;
        }

        log_info("poweroff flag:%d cnt:%d\n", goto_poweroff_flag, goto_poweroff_cnt);

        if (goto_poweroff_flag) {
            goto_poweroff_cnt++;
            if (goto_poweroff_cnt >= POWER_OFF_CNT) {
                goto_poweroff_cnt = 0;
                sys_enter_soft_poweroff(POWEROFF_NORMAL);
            }
        }
        break;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   poweroff 立刻关机
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void power_off_instantly()
{
    if (goto_poweroff_first_flag == 0) {
        goto_poweroff_first_flag = 1;
    } else {
        puts("power_off_instantly had call\n");
        return;
    }

    sys_enter_soft_poweroff(POWEROFF_NORMAL);
}



static void app_idle_enter_softoff(void)
{
    //power off前的ui处理
    power_off_wait_ui();

#if TCFG_CHARGE_ENABLE
    if (get_lvcmp_det() && (0 == get_charge_full_flag())) {
        log_info("charge inset, system reset!\n");
        cpu_reset();
    }
#endif

    power_set_soft_poweroff();
}

struct app_mode *app_enter_idle_mode(int arg)
{
    int msg[16];
    struct app_mode *next_mode;

    app_idle_init(arg);

    while (1) {
        if (!app_get_message(msg, ARRAY_SIZE(msg), idle_mode_key_table)) {
            continue;
        }
        next_mode = app_mode_switch_handler(msg);
        if (next_mode) {
            break;
        }

        switch (msg[0]) {
        case MSG_FROM_APP:
            idle_app_msg_handler(msg + 1);
            break;
        case MSG_FROM_DEVICE:
            break;
        }

        app_default_msg_handler(msg);
    }

    app_idle_exit();

    return next_mode;
}

void app_power_off(void *priv)
{
    app_idle_enter_softoff();
}

static int app_power_off_tone_cb(void *priv, enum stream_event event)
{
    if (event == STREAM_EVENT_STOP) {
        app_idle_enter_softoff();
    }
    return 0;
}

static void device_online_timeout(void *priv)
{
    int ret = -1;//play_tone_file_callback(get_tone_files()->power_off, NULL,pp_power_off_tone_cb);
    wait_device_online_timer = 0;
    printf("power_off tone play ret:%d", ret);
    if (ret) {
        if (app_var.goto_poweroff_flag) {
            log_info("power_off tone play err,enter soft poweroff");
            app_idle_enter_softoff();
        }
    }
}

static int app_idle_init(int param)
{
    log_info("idle_mode_enter: %d\n", param);

    idle_enter_param = param;

    switch (param) {
    case IDLE_MODE_PLAY_POWEROFF:
        if (app_var.goto_poweroff_flag) {
            syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 2);
            //如果开启了VM配置项暂存RAM功能则在关机前保存数据到vm_flash
            if (get_vm_ram_storage_enable() || get_vm_ram_storage_in_irq_enable()) {
                vm_flush2flash(1);
            }
            os_taskq_flush();
            int ret = -1;//play_tone_file_callback(get_tone_files()->power_off, NULL,app_power_off_tone_cb);
            printf("power_off tone play ret:%d", ret);
            if (ret) {
                if (app_var.goto_poweroff_flag) {
                    log_info("power_off tone play err,enter soft poweroff");
                    app_idle_enter_softoff();
                }
            }
        }
        break;
    case IDLE_MODE_WAIT_POWEROFF:
        os_taskq_flush();
        syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 2);
        break;
    case IDLE_MODE_CHARGE:
        break;
    case IDLE_MODE_WAIT_DEVONLINE:      //等待1s设备没有上线就进入关机
        if (wait_device_online_timer == 0) {
            wait_device_online_timer = sys_timeout_add(NULL, device_online_timeout, 1000);
        }
        break;
    }

    app_send_message(APP_MSG_ENTER_MODE, APP_MODE_IDLE);
    return 0;
}

void app_idle_exit()
{
    if (wait_device_online_timer) {
        sys_timeout_del(wait_device_online_timer);
        wait_device_online_timer = 0;
    }
    app_send_message(APP_MSG_EXIT_MODE, APP_MODE_IDLE);
}

static int idle_mode_try_enter(int arg)
{
    return 0;
}

static int idle_mode_try_exit()
{
    return 0;
}

static const struct app_mode_ops idle_mode_ops = {
    .try_enter          = idle_mode_try_enter,
    .try_exit           = idle_mode_try_exit,
};

REGISTER_APP_MODE(idle_mode) = {
    .name   = APP_MODE_IDLE,
    .index  = 0xff,
    .ops    = &idle_mode_ops,
};
