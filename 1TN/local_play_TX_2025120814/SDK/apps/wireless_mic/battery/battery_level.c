#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".battery_level.data.bss")
#pragma data_seg(".battery_level.data")
#pragma const_seg(".battery_level.text.const")
#pragma code_seg(".battery_level.text")
#endif
#include "system/includes.h"
#include "battery_manager.h"
#include "app_power_manage.h"
#include "app_main.h"
#include "app_config.h"
#include "app_action.h"
#include "asm/charge.h"
#include "app_tone.h"
#include "gpadc.h"
#include "btstack/avctp_user.h"
#include "user_cfg.h"
#include "asm/charge.h"
#include "bt_tws.h"
#include "idle.h"

#define LOG_TAG             "[BATTERY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

enum {
    VBAT_NORMAL = 0,
    VBAT_WARNING,
    VBAT_LOWPOWER,
} VBAT_STATUS;

#define VBAT_DETECT_CNT         6 //每次更新电池电量采集的次数
#define VBAT_DETECT_TIME        10L //每次采集的时间间隔
#define VBAT_UPDATE_SLOW_TIME   60000L //慢周期更新电池电量
#define VBAT_UPDATE_FAST_TIME   10000L //快周期更新电池电量

union battery_data {
    u32 raw_data;
    struct {
        u8 reserved;
        u16 voltage;
        u8 percent;
    } __attribute__((packed)) data;
};

static u16 vbat_slow_timer = 0;
static u16 vbat_fast_timer = 0;
static u16 lowpower_timer = 0;
static u8 old_battery_level = 9;
static u16 cur_battery_voltage = 0;
static u8 cur_battery_level = 0;
static u8 cur_battery_percent = 0;
static u8 tws_sibling_bat_level = 0xff;
static u8 tws_sibling_bat_percent_level = 0xff;
static u8 cur_bat_st = VBAT_NORMAL;
static u8 battery_curve_max;
static struct battery_curve *battery_curve_p = NULL;

void vbat_check(void *priv);
void clr_wdt(void);

static void power_warning_timer(void *p)
{
    batmgr_send_msg(POWER_EVENT_POWER_WARNING, 0);
}

static int app_power_event_handler(int *msg)
{
    int ret = false;

#if(TCFG_SYS_LVD_EN == 1)
    switch (msg[0]) {
    case POWER_EVENT_POWER_NORMAL:
        break;
    case POWER_EVENT_POWER_WARNING:
        play_tone_file(get_tone_files()->low_power);
        if (lowpower_timer == 0) {
            lowpower_timer = sys_timer_add(NULL, power_warning_timer, LOW_POWER_WARN_TIME);
        }
        break;
    case POWER_EVENT_POWER_LOW:
        r_printf(" POWER_EVENT_POWER_LOW");
        vbat_timer_delete();
        if (lowpower_timer) {
            sys_timer_del(lowpower_timer);
            lowpower_timer = 0 ;
        }
        if (!app_in_mode(APP_MODE_IDLE)) {
            sys_enter_soft_poweroff(POWEROFF_NORMAL);
        } else {
            power_set_soft_poweroff();
        }
        break;
    case POWER_EVENT_POWER_CHANGE:
        /* log_info("POWER_EVENT_POWER_CHANGE\n"); */

        if (!app_in_mode(APP_MODE_BT)) {
            break;
        }
        break;
    case POWER_EVENT_POWER_CHARGE:
        if (lowpower_timer) {
            sys_timer_del(lowpower_timer);
            lowpower_timer = 0 ;
        }
        break;
#if TCFG_CHARGE_ENABLE
    case CHARGE_EVENT_LDO5V_OFF:
        //充电拔出时重新初始化检测定时器
        vbat_check_init();
        break;
#endif
    default:
        break;
    }
#endif

    return ret;
}
APP_MSG_HANDLER(bat_level_msg_entry) = {
    .owner      = 0xff,
    .from       = MSG_FROM_BATTERY,
    .handler    = app_power_event_handler,
};

static u16 get_vbat_voltage(void)
{
    return gpadc_battery_get_voltage();
}

static u16 battery_calc_percent(u16 bat_val)
{
    u8 i, tmp_percent;
    u16 max, min, div_percent;
    if (battery_curve_p == NULL) {
        log_error("battery_curve not init!!!\n");
        return 0;
    }
#if (TCFG_BATTERY_CURVE_ENABLE && TCFG_CHARGE_ENABLE)
    if (IS_CHARGE_EN()) {
        for (i = 0; i < (battery_curve_max - 1); i++) {
            if (bat_val <= battery_curve_p[i].chargeing_voltage) {
                return battery_curve_p[i].percent;
            }
            if (bat_val >= battery_curve_p[i + 1].chargeing_voltage) {
                continue;
            }
            div_percent = battery_curve_p[i + 1].percent - battery_curve_p[i].percent;
            min = battery_curve_p[i].chargeing_voltage;
            max = battery_curve_p[i + 1].chargeing_voltage;
            tmp_percent = battery_curve_p[i].percent;
            tmp_percent += (bat_val - min) * div_percent / (max - min);
            return tmp_percent;
        }
    } else
#endif
    {
        for (i = 0; i < (battery_curve_max - 1); i++) {
            if (bat_val <= battery_curve_p[i].discharge_voltage) {
                return battery_curve_p[i].percent;
            }
            if (bat_val >= battery_curve_p[i + 1].discharge_voltage) {
                continue;
            }
            div_percent = battery_curve_p[i + 1].percent - battery_curve_p[i].percent;
            min = battery_curve_p[i].discharge_voltage;
            max = battery_curve_p[i + 1].discharge_voltage;
            tmp_percent = battery_curve_p[i].percent;
            tmp_percent += (bat_val - min) * div_percent / (max - min);
            return tmp_percent;
        }
    }
    return battery_curve_p[battery_curve_max - 1].percent;
}

u16 get_vbat_value(void)
{
    return cur_battery_voltage;
}

u8 get_vbat_percent(void)
{
    return cur_battery_percent;
}

bool get_vbat_need_shutdown(void)
{
    if ((cur_battery_voltage <= app_var.poweroff_tone_v) || adc_check_vbat_lowpower()) {
        return TRUE;
    }
    return FALSE;
}

//将当前电量转换为1~9级发送给手机同步电量
u8  battery_value_to_phone_level(void)
{
    u8  battery_level = 0;
    u8 vbat_percent = get_vbat_percent();
    if (vbat_percent < 5) { //小于5%电量等级为0，显示10%
        return 0;
    }
    battery_level = (vbat_percent - 5) / 10;
    return battery_level;
}

//获取自身的电量
u8  get_self_battery_level(void)
{
    return cur_battery_level;
}

u8 get_cur_battery_level(void)
{
    return cur_battery_level;
}

void vbat_check_slow(void *priv)
{
    if (vbat_fast_timer == 0) {
        vbat_fast_timer = usr_timer_add(NULL, vbat_check, VBAT_DETECT_TIME, 1);
    }
    if (get_charge_online_flag()) {
        sys_timer_modify(vbat_slow_timer, VBAT_UPDATE_SLOW_TIME);
    } else {
        sys_timer_modify(vbat_slow_timer, VBAT_UPDATE_FAST_TIME);
    }
}

void vbat_curve_init(const struct battery_curve *curve_table, int table_size)
{
    battery_curve_p = (struct battery_curve *)curve_table;
    battery_curve_max = table_size;

    //初始化相关变量
    cur_battery_voltage = get_vbat_voltage();
    cur_battery_percent = battery_calc_percent(cur_battery_voltage);
    cur_battery_level = battery_value_to_phone_level();
}

void vbat_check_init(void)
{
#if TCFG_BATTERY_CURVE_ENABLE == 0
    u8 tmp[128] = {0};
    int i;
    u16 battery_0, battery_100;
    union battery_data battery_data_t;

    //初始化电池曲线
    if (battery_curve_p == NULL) {
        memset(tmp, 0x00, sizeof(tmp));
        int ret = syscfg_read(CFG_BATTERY_CURVE_ID, tmp, sizeof(tmp));
        if (ret > 0) {
            battery_curve_max = ret / sizeof(battery_data_t);
        } else {
            battery_curve_max = 2;
        }
        battery_curve_p = malloc(battery_curve_max * sizeof(struct battery_curve));
        ASSERT(battery_curve_p, "malloc battery_curve err!");
        if (ret < 0) {
            log_error("battery curve id, ret: %d\n", ret);
            battery_0 = app_var.poweroff_tone_v;
#if TCFG_CHARGE_ENABLE
            //防止部分电池充不了这么高电量，充满显示未满的情况
            battery_100 = (get_charge_full_value() - 100);
#else
            battery_100 = 4100;
#endif
            battery_curve_p[0].percent = 0;
            battery_curve_p[0].discharge_voltage = battery_0;
            battery_curve_p[1].percent = 100;
            battery_curve_p[1].discharge_voltage = battery_100;
            log_info("percent: %d, voltage: %d mV", 0, battery_curve_p[0].discharge_voltage);
            log_info("percent: %d, voltage: %d mV", 100, battery_curve_p[1].discharge_voltage);
        } else {
            for (i = 0; i < battery_curve_max; i++) {
                memcpy(&battery_data_t.raw_data,
                       &tmp[i * sizeof(battery_data_t)], sizeof(battery_data_t));
                battery_curve_p[i].percent = battery_data_t.data.percent;
                battery_curve_p[i].discharge_voltage = battery_data_t.data.voltage;
                log_info("percent: %d, voltage: %d mV\n",
                         battery_curve_p[i].percent, battery_curve_p[i].discharge_voltage);
            }
        }
        //初始化相关变量
        cur_battery_voltage = get_vbat_voltage();
        cur_battery_percent = battery_calc_percent(cur_battery_voltage);
        cur_battery_level = battery_value_to_phone_level();
    }
#endif

    if (vbat_slow_timer == 0) {
        vbat_slow_timer = sys_timer_add(NULL, vbat_check_slow, VBAT_UPDATE_FAST_TIME);
    } else {
        sys_timer_modify(vbat_slow_timer, VBAT_UPDATE_FAST_TIME);
    }

    if (vbat_fast_timer == 0) {
        vbat_fast_timer = usr_timer_add(NULL, vbat_check, VBAT_DETECT_TIME, 1);
    }
}

void vbat_timer_delete(void)
{
    if (vbat_slow_timer) {
        sys_timer_del(vbat_slow_timer);
        vbat_slow_timer = 0;
    }
    if (vbat_fast_timer) {
        usr_timer_del(vbat_fast_timer);
        vbat_fast_timer = 0;
    }
}




////////////////// USER VBAT DISPLAY /////////////////////////////
extern u8 hundreds;	//百位，0不显示，1仅百分比，2百分比+供电，3-百分比+百位，4全部显示
extern u8 tens; 		//十位，10-F，11-不显示
extern u8 unit;		//个位，10-F，11-不显示
//static u16 curr_vbat_value = 0;
extern void led7_charge_flash_add();

static int user_vbat_percent = 0;
static u16 vbat_cnt = 0;

static int curr_vbat_value = 0;
static int really_vbat_value = 0;
void user_vbat_init()
{
    curr_vbat_value = get_vbat_percent();
    really_vbat_value = curr_vbat_value;
    if(curr_vbat_value >= 100)
    {
        hundreds = 3;
        tens = 0;
        unit = 0;
    }
    else{
        hundreds = 0;
        tens = curr_vbat_value / 10;
        unit = curr_vbat_value % 10;
    }
    if(really_vbat_value <= 10){
        if(!get_charge_online_flag())
            led7_charge_flash_add();
    }
}

void really_vbat_display()
{
    if(really_vbat_value >= 100)
    {
        hundreds = 3;
        tens = 0;
        unit = 0;
    }
    else{
        hundreds = 0;
        tens = really_vbat_value / 10;
        unit = really_vbat_value % 10;
    }
}

void user_vbat_display()
{
    user_vbat_percent += get_vbat_percent();
    vbat_cnt++;
    if(vbat_cnt ==10)
    {
        user_vbat_percent = user_vbat_percent / 10;
        printf("*********** user_vbat_percent = %d ***********\n",user_vbat_percent);
        if((really_vbat_value - user_vbat_percent >= 2) || (really_vbat_value < user_vbat_percent))
        {
            curr_vbat_value = user_vbat_percent;
            if(really_vbat_value > curr_vbat_value)
            {
                really_vbat_value--;
            }
            //user_set_real_charge_time_add();
            printf("********** really_vbat_value = %d   curr_vbat_value = %d **********\n",really_vbat_value,curr_vbat_value);
            /*if(really_vbat_value == curr_vbat_value)
            {
                user_set_real_charge_time_del();
            }*/
            really_vbat_display();
            vbat_cnt = 0;
            user_vbat_percent = 0;
        }
        else
        {
            really_vbat_value = user_vbat_percent;
            really_vbat_display();
            //really_vbat_value = user_vbat_percent;
            vbat_cnt = 0;
            user_vbat_percent = 0;
        }
        if(really_vbat_value <= 10)
        {
            led7_charge_flash_add();
        }

    }
    else{
        return ;
    }

}



static u16 user_vbat_display_time = 0;
void user_vbat_display_add()
{
    if(!user_vbat_display_time)
    {
        user_vbat_display_time = sys_timer_add(NULL,user_vbat_display,3200);
    }
}

void user_vbat_display_del()
{
    if(user_vbat_display_time)
    {
        sys_timer_del(user_vbat_display_time);
        user_vbat_display_time = 0;
    }
}



/////////////////充电电量显示////////////////////////////////////


static u16 charge_vbat_percent = 0;
static u16 charge_vbat_cnt = 0;


#define USER_DEFINE_REALLY_INTERAL  20


void user_set_real_charge_time()
{
    if(really_vbat_value < curr_vbat_value)
    {
        really_vbat_value++;
    }
}

static u16 really_time = 0;
void user_set_real_charge_time_add()
{
    if(!really_time)
    {
        really_time = sys_timer_add(NULL,user_set_real_charge_time,1000 * USER_DEFINE_REALLY_INTERAL);
    }
}

void user_set_real_charge_time_del()
{
    if(really_time)
    {
        sys_timer_del(really_time);
        really_time = 0;
    }
}

extern void led7_charge_flash_del();
void user_charge_vbat_display()
{
    charge_vbat_percent += get_vbat_percent();
    charge_vbat_cnt++;
    if(charge_vbat_cnt ==10)
    {
        charge_vbat_percent = charge_vbat_percent / 10;
        printf("*********** charge_vbat_percent = %d ***********\n",charge_vbat_percent);
        if((charge_vbat_percent - really_vbat_value) >= 2 || really_vbat_value > charge_vbat_percent)
        {
            curr_vbat_value = charge_vbat_percent;
            if(really_vbat_value < curr_vbat_value)
            {
                really_vbat_value++;
            }
            //user_set_real_charge_time_add();
            printf("********** really_vbat_value = %d   curr_vbat_value = %d **********\n",really_vbat_value,curr_vbat_value);
            /*if(really_vbat_value == curr_vbat_value)
            {
                user_set_real_charge_time_del();
            }*/
            if(really_vbat_value >= 100)
            {
                hundreds = 3;
                tens = 0;
                unit = 0;
                led7_charge_flash_del();
            }
            else{
                hundreds = 0;
                tens = really_vbat_value / 10;
                unit = really_vbat_value % 10;
            }
            charge_vbat_cnt = 0;
            charge_vbat_percent = 0;
        }
        else
        {
            if(really_vbat_value != 100)
                really_vbat_value = charge_vbat_percent;

            if(really_vbat_value >= 100)
            {
                hundreds = 3;
                tens = 0;
                unit = 0;
                led7_charge_flash_del();
            }
            else{
                hundreds = 0;
                tens = really_vbat_value / 10;
                unit = really_vbat_value % 10;
            }

            charge_vbat_cnt = 0;
            charge_vbat_percent = 0;
        }

    }
    else{
        return ;
    }

}

static u16 charge_dete_time = 0;
void user_charge_vbat_display_add()
{
    if(!charge_dete_time)
    {
        charge_dete_time = sys_timer_add(NULL,user_charge_vbat_display,3000);
    }
}


void user_charge_vbat_display_del()
{
    if(charge_dete_time)
    {
        sys_timer_del(charge_dete_time);
        charge_dete_time = 0;
    }
}

/////////////////////////////////////////////////////////////////



void vbat_check(void *priv)
{
    static u8 unit_cnt = 0;
    static u8 low_voice_cnt = 0;
    static u8 low_power_cnt = 0;
    static u8 power_normal_cnt = 0;
    static u8 charge_online_flag = 0;
    static u8 low_voice_first_flag = 1;//进入低电后先提醒一次
    static u32 bat_voltage = 0;
    u16 tmp_percent;

    bat_voltage += get_vbat_voltage();
    unit_cnt++;
    if (unit_cnt < VBAT_DETECT_CNT) {
        return;
    }
    unit_cnt = 0;

    //更新电池电压,以及电池百分比,还有电池等级
    cur_battery_voltage = bat_voltage / VBAT_DETECT_CNT;
    bat_voltage = 0;
    tmp_percent = battery_calc_percent(cur_battery_voltage);
    if (get_charge_online_flag()) {
        if (tmp_percent > cur_battery_percent) {
            cur_battery_percent++;
        }
    } else {
        if (tmp_percent < cur_battery_percent) {
            cur_battery_percent--;
        }
    }
    cur_battery_level = battery_value_to_phone_level();

    /* g_printf("cur_voltage: %d mV, tmp_percent: %d, cur_percent: %d, cur_level: %d\n", */
    /*          cur_battery_voltage, tmp_percent, cur_battery_percent, cur_battery_level); */

    if (get_charge_online_flag() == 0) {
        if (adc_check_vbat_lowpower() ||
            (cur_battery_voltage <= app_var.poweroff_tone_v)) { //低电关机
            low_power_cnt++;
            low_voice_cnt = 0;
            power_normal_cnt = 0;
            cur_bat_st = VBAT_LOWPOWER;
            if (low_power_cnt > 6) {
                log_info("\n*******Low Power,enter softpoweroff********\n");
                low_power_cnt = 0;
                batmgr_send_msg(POWER_EVENT_POWER_LOW, 0);
                usr_timer_del(vbat_fast_timer);
                vbat_fast_timer = 0;
            }
        } else if (cur_battery_voltage <= app_var.warning_tone_v) { //低电提醒
            low_voice_cnt ++;
            low_power_cnt = 0;
            power_normal_cnt = 0;
            cur_bat_st = VBAT_WARNING;
            if ((low_voice_first_flag && low_voice_cnt > 1) || //第一次进低电10s后报一次
                (!low_voice_first_flag && low_voice_cnt >= 5)) {
                low_voice_first_flag = 0;
                low_voice_cnt = 0;
#if(TCFG_SYS_LVD_EN == 1)
                if (!lowpower_timer) {
                    log_info("\n**Low Power,Please Charge Soon!!!**\n");
                    batmgr_send_msg(POWER_EVENT_POWER_WARNING, 0);
                }
#endif
            }
        } else {
            power_normal_cnt++;
            low_voice_cnt = 0;
            low_power_cnt = 0;
            if (power_normal_cnt > 2) {
                if (cur_bat_st != VBAT_NORMAL) {
                    log_info("[Noraml power]\n");
                    cur_bat_st = VBAT_NORMAL;
                    batmgr_send_msg(POWER_EVENT_POWER_NORMAL, 0);
                }
            }
        }
    } else {
        batmgr_send_msg(POWER_EVENT_POWER_CHARGE, 0);
    }

    if (cur_bat_st != VBAT_LOWPOWER) {
        usr_timer_del(vbat_fast_timer);
        vbat_fast_timer = 0;
        //电量等级变化,或者在仓状态变化,交换电量
        if ((cur_battery_level != old_battery_level) ||
            (charge_online_flag != get_charge_online_flag())) {
            batmgr_send_msg(POWER_EVENT_POWER_CHANGE, 0);
        }
        charge_online_flag =  get_charge_online_flag();
        old_battery_level = cur_battery_level;
    }
}

bool vbat_is_low_power(void)
{
    return (cur_bat_st != VBAT_NORMAL);
}

void check_power_on_voltage(void)
{
#if(TCFG_SYS_LVD_EN == 1)

    u16 val = 0;
    u8 normal_power_cnt = 0;
    u8 low_power_cnt = 0;

    while (1) {
        clr_wdt();
        val = get_vbat_voltage();
        printf("vbat: %d\n", val);
        if ((val < app_var.poweroff_tone_v) || adc_check_vbat_lowpower()) {
            low_power_cnt++;
            normal_power_cnt = 0;
            if (low_power_cnt > 10) {
                /* ui_update_status(STATUS_POWERON_LOWPOWER); */
                os_time_dly(100);
                log_info("power on low power , enter softpoweroff!\n");
                power_set_soft_poweroff();
            }
        } else {
            normal_power_cnt++;
            low_power_cnt = 0;
            if (normal_power_cnt > 10) {
                vbat_check_init();
                return;
            }
        }
    }
#endif
}
