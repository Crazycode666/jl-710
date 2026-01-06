#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".bt_key_func.data.bss")
#pragma data_seg(".bt_key_func.data")
#pragma const_seg(".bt_key_func.text.const")
#pragma code_seg(".bt_key_func.text")
#endif
#include "key_driver.h"
#include "audio_manager.h"
#include "app_main.h"
#include "audio_config.h"
#include "bt_key_func.h"
#include "ui/ui_api.h"
#include "ui_manage.h"
#include "bt_event_func.h"
#include "app_tone.h"
#include "node_param_update.h"
#include "le_broadcast.h"
#include "node_uuid.h"
#include "effects/effects_adj.h"
#include "effects/audio_llns_dns.h"
#include "effects/audio_llns.h"
#include "clock_manager/clock_manager.h"

/* --------------------------------------------------------------------------*/
/**
 * @brief 切换reverb参数
 *
 * @param bypass 0xff:切换参数，1：bypass，0：关闭bypass
 */
/* ----------------------------------------------------------------------------*/
void app_plate_reverb_parm_switch(int bypass)
{
    static u8 cfg_index = 0;
    //没有bypass，则切换参数
    if (0xff == bypass) {
        cfg_index++;
    }
    g_printf("cfg_index:%d", cfg_index);
    //第二个参数需要与音频框图对应节点名称一致
    int ret = plate_reverb_update_parm_base(0, "PlateRever51011", cfg_index, bypass);
    if (-1 == ret) {
        //读不到节点参数，可能index已经超过可读参数
        cfg_index = 0;
        plate_reverb_update_parm_base(0, "PlateRever51011", cfg_index, bypass);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 切换llns_dns参数
 *
 * @param bypass 0xff:切换参数，1：bypass，0：关闭bypass
 */
/* ----------------------------------------------------------------------------*/
void app_llns_dns_parm_switch(int bypass)
{
    static u8 cfg_index = 0;
    //没有bypass，则切换参数
    if (0xff == bypass) {
        cfg_index++;
    } else {
#if 0//(LEA_BIG_FIX_ROLE == LEA_ROLE_AS_TX)
        if (!bypass) {
            clock_alloc("max", TCFG_FIX_CLOCK_FREQ);
        }
#endif
        //重新开关数据流是为了重设延时,重启数据流时会通过get_llns_dns_bypass_status获取bypass状态
        broadcast_audio_all_close(get_global_big_hdl());
        broadcast_audio_all_open(get_global_big_hdl());
#if 0//(LEA_BIG_FIX_ROLE == LEA_ROLE_AS_TX)
        if (bypass) {
            clock_free("max");
        }
#endif
        return;
    }
    g_printf("cfg_index:%d", cfg_index);
    //第二个参数需要与音频框图对应节点名称一致
    int ret = llns_dns_update_parm_base(0, "LLNS_DNS1", cfg_index, bypass);
    if (-1 == ret) {
        //读不到节点参数，可能index已经超过可读参数
        cfg_index = 0;
        llns_dns_update_parm_base(0, "LLNS_DNS1", cfg_index, bypass);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 切换llns参数
 *
 * @param bypass 0xff:切换参数，1：bypass，0：关闭bypass
 */
/* ----------------------------------------------------------------------------*/
void app_llns_parm_switch(int bypass)
{
    static u8 cfg_index = 0;
    int ret = 0;
    llns_param_tool_set cfg = {0};

    //没有bypass，则切换参数
    if (0xff == bypass) {
        cfg_index++;
        g_printf("llns cfg_index:%d", cfg_index);
        ret = jlstream_read_form_data(0, "LLNS1", cfg_index, &cfg);
        if (!ret) {
            cfg_index = 0; //读不到了从头开始
            ret = jlstream_read_form_data(0, "LLNS1", cfg_index, &cfg);
            if (!ret) {
                printf("read parm err, %s, %s\n", __func__, "LLNS1");
                return;
            }
        }
    } else {
        ret = jlstream_read_form_data(0, "LLNS1", cfg_index, &cfg);
        if (!ret) {
            printf("read parm err, %s, %s\n", __func__, "LLNS1");
            return;
        }
        //第二个参数需要与音频框图对应节点名称一致
        cfg.is_bypass = bypass;//重写by_pass状态
    }


    ret = jlstream_set_node_param(NODE_UUID_LLNS, "LLNS1", &cfg, sizeof(cfg));
    if (!ret) {
        printf("set parm err, %s, %s\n", __func__, "LLNS1");
    }

}
