#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "app_msg.h"
#include "app_mode_manager/app_mode_manager.h"
#include "app_mode.h"
#include "poweroff.h"
#include "app_config.h"
#include "app_music.h"
#include "app_default_msg_handler.h"
#include "bt_background.h"
#include "soundbox.h"

enum {
    SYS_POWERON_BY_KEY = 1,
    SYS_POWERON_BY_OUT_BOX,
};

enum {
    SYS_POWEROFF_BY_KEY = 1,
    SYS_POWEROFF_BY_TIMEOUT,
};

typedef struct _APP_VAR {
    u8 volume_def_state;
    s16 music_volume;
    s16 call_volume;
    s16 wtone_volume;
    s16 ktone_volume;
    s16 ring_volume;
    u8 aec_dac_gain;
    u8 aec_mic_gain;
    u8 aec_mic1_gain;
    u8 rf_power;
    u8 goto_poweroff_flag;
    u16 goto_poweroff_cnt;
    u8 play_poweron_tone;
    u8 poweron_reason;
    u8 poweroff_reason;
    u16 auto_off_time;
    u16 warning_tone_v;
    u16 poweroff_tone_v;
    s16 mic_eff_volume;
} APP_VAR;

struct bt_mode_var {
    u8 init_start; //蓝牙协议栈已经开始初始化标志位
    u8 init_ok; //蓝牙初始化完成标志
    u8 initializing; //蓝牙正在初始化标志
    u8 exiting; //蓝牙正在退出
};

extern APP_VAR app_var;
extern struct bt_mode_var g_bt_hdl;

enum app_mode_t {
    APP_MODE_IDLE,
    APP_MODE_UPDATE,
    APP_MODE_POWERON,
    APP_MODE_BT,
    APP_MODE_MUSIC,
    APP_MODE_FM,
    APP_MODE_RECORD,
    APP_MODE_LINEIN,
    APP_MODE_RTC,
    APP_MODE_PC,
    APP_MODE_SPDIF,
    APP_MODE_IIS,
    APP_MODE_MIC,
    APP_MODE_SINK,
    APP_MODE_NULL,
};

enum app_mode_index {
    APP_MODE_BT_INDEX,
};

void app_power_off(void *priv);

struct app_mode *app_mode_switch_handler(int *msg);

#endif
