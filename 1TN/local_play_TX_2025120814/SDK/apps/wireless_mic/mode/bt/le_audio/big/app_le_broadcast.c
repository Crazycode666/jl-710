/*********************************************************************************************
    *   Filename        : app_le_broadcast.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:18

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"
#include "le_broadcast.h"
#include "app_le_broadcast.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "app_tone.h"
#include "app_main.h"
#include "wireless_trans.h"
#include "audio_config.h"
#include "soundbox.h"
#include "wireless_trans_manager.h"

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))

/**************************************************************************************************
  Macros
**************************************************************************************************/
#define LOG_TAG             "[APP_LE_BROADCAST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/
enum {
    ///0x1000起始为了不要跟提示音的IDEX_TONE_重叠了
    TONE_INDEX_BROADCAST_OPEN = 0x1000,
    TONE_INDEX_BROADCAST_CLOSE,
};

struct app_bis_hdl_t {
    u8 bis_status;
    u8 remote_dev;
    u16 bis_hdl;
    u16 aux_hdl;
    audio_param_t enc;
};

struct app_big_hdl_t {
    u8 used;
    u8 big_status;
    u8 big_hdl;
    u8 local_dev;
    struct app_bis_hdl_t bis_hdl_info[BIG_MAX_BIS_NUMS];
};

/**************************************************************************************************
  Local Function Prototypes
**************************************************************************************************/
static void broadcast_pair_tx_event_callback(const PAIR_EVENT event, void *priv);
static void broadcast_pair_rx_event_callback(const PAIR_EVENT event, void *priv);
static int broadcast_padv_data_deal(void *priv);

/**************************************************************************************************
  Local Global Variables
**************************************************************************************************/
static OS_MUTEX mutex;
static u8 bis_connected_nums = 0;
static struct broadcast_sync_info app_broadcast_data_sync;  /*!< 用于记录广播同步状态 */
static struct app_big_hdl_t app_big_hdl_info[BIG_MAX_NUMS];
static u8 receiver_connected_status = 0; /*< 广播接收端连接状态 */
static u8 app_broadcast_init_flag = 0;
static const pair_callback_t pair_tx_cb = {
    .pair_event_cb = broadcast_pair_tx_event_callback,
};
static const pair_callback_t pair_rx_cb = {
    .pair_event_cb = broadcast_pair_rx_event_callback,
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* --------------------------------------------------------------------------*/
/**
 * @brief 申请互斥量，用于保护临界区代码，与app_broadcast_mutex_post成对使用
 *
 * @param mutex:已创建的互斥量指针变量
 */
/* ----------------------------------------------------------------------------*/
static inline void app_broadcast_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    if (!app_broadcast_init_flag) {
        log_error("%s err, mutex uninit", __FUNCTION__);
        return;
    }

    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 用于外部重新打开发送端的数据流
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_reset_transmitter(void)
{
    int i = 0;
    if (get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            //固定收发角色重启广播数据流
            broadcast_audio_recorder_reset(app_big_hdl_info[i].big_hdl);
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 用于外部打开发送端的数据流
 */
/* ----------------------------------------------------------------------------*/
bool app_broadcast_open_transmitter(void)
{
    u8 i;
    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].big_status == APP_BROADCAST_STATUS_START && get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) {
            broadcast_audio_recorder_open(app_big_hdl_info[i].big_hdl);
            return true;
        }
    }
    return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 用于外部关闭发送端的数据流
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_close_transmitter(void)
{
    int i = 0;
    if (get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            //固定收发角色关闭广播数据流
            broadcast_audio_recorder_close(app_big_hdl_info[i].big_hdl);
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 释放互斥量，用于保护临界区代码，与app_broadcast_mutex_pend成对使用
 *
 * @param mutex:已创建的互斥量指针变量
 */
/* ----------------------------------------------------------------------------*/
static inline void app_broadcast_mutex_post(OS_MUTEX *mutex, u32 line)
{
    if (!app_broadcast_init_flag) {
        log_error("%s err, mutex uninit", __FUNCTION__);
        return;
    }

    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief BIG开关提示音结束回调接口
 *
 * @param priv:传递的参数
 * @param event:提示音回调事件
 *
 * @return
 */
/* ----------------------------------------------------------------------------*/
static int broadcast_tone_play_end_callback(void *priv, enum stream_event event)
{
    u32 index = (u32)priv;

    g_printf("%s, event:0x%x", __FUNCTION__, event);

    switch (event) {
    case STREAM_EVENT_NONE:
    case STREAM_EVENT_STOP:
        switch (index) {
        case TONE_INDEX_BROADCAST_OPEN:
            g_printf("TONE_INDEX_BROADCAST_OPEN");
            break;
        case TONE_INDEX_BROADCAST_CLOSE:
            g_printf("TONE_INDEX_BROADCAST_CLOSE");
            break;
        default:
            break;
        }
    default:
        break;
    }

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取接收方连接状态
 *
 * @return 接收方连接状态
 */
/* ----------------------------------------------------------------------------*/
u8 get_receiver_connected_status(void)
{
    return receiver_connected_status;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief BIG状态事件处理函数
 *
 * @param msg:状态事件附带的返回参数
 *
 * @return
 */
/* ----------------------------------------------------------------------------*/
static int app_broadcast_conn_status_event_handler(int *msg)
{
    u8 i, j;
    u8 find = 0;
    u8 big_hdl;
    int ret;
    big_hdl_t *hdl;
    int *event = msg;
    int result = 0;
    u32 pair_code = 0;

    /* g_printf("%s, event:0x%x", __FUNCTION__, event[0]); */

    switch (event[0]) {
    case BIG_EVENT_TRANSMITTER_CONNECT:
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_TX_EN)
        g_printf("BIG_EVENT_TRANSMITTER_CONNECT");
        //由于是异步操作需要加互斥量保护，避免broadcast_close的代码与其同时运行,添加的流程请放在互斥量保护区里面
        app_broadcast_mutex_pend(&mutex, __LINE__);

        wireless_mic_audio_status_reset();
        //关闭VM写RAM
        vm_ram_direct_2_flash_switch(FALSE);

        hdl = (big_hdl_t *)&event[1];

        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (app_big_hdl_info[i].used && (app_big_hdl_info[i].big_hdl == hdl->big_hdl)) {
                app_big_hdl_info[i].local_dev = WIRELESS_TX_DEV1;
                for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
                    if (!app_big_hdl_info[i].bis_hdl_info[j].bis_hdl)     {
                        app_big_hdl_info[i].bis_hdl_info[j].bis_hdl = hdl->bis_hdl[0];
                        app_big_hdl_info[i].bis_hdl_info[j].aux_hdl = hdl->aux_hdl;
                        app_big_hdl_info[i].bis_hdl_info[j].remote_dev = WIRELESS_RX_DEV1;
                        memcpy(&app_big_hdl_info[i].bis_hdl_info[j].enc, &hdl->enc, sizeof(audio_param_t));
                        find = 1;
                        break;
                    }
                }
            }
            if (find) {
                break;
            }
        }

        if (!find) {
            //释放互斥量
            app_broadcast_mutex_post(&mutex, __LINE__);
            break;
        }

#if TCFG_AUTO_SHUT_DOWN_TIME
        sys_auto_shut_down_disable();
#endif

        bis_connected_nums++;
        ASSERT(bis_connected_nums <= BIG_MAX_BIS_NUMS && bis_connected_nums >= 0, "bis_connected_nums:%d", bis_connected_nums);
        if (bis_connected_nums == 1) {
            app_send_message(APP_MSG_LE_FIRST_CONNECT, 0);
        } else {
            app_send_message(APP_MSG_LE_SECOND_CONNECT, 0);
        }
        ret = broadcast_transmitter_connect_deal((void *)hdl);
        if (ret < 0) {
            r_printf("broadcast_transmitter_connect_deal fail");
        }

        //释放互斥量
        app_broadcast_mutex_post(&mutex, __LINE__);
#endif
        break;

    case BIG_EVENT_TRANSMITTER_DISCONNECT:

        wireless_mic_audio_status_reset();
        bis_connected_nums--;
        ASSERT(bis_connected_nums <= BIG_MAX_BIS_NUMS && bis_connected_nums >= 0, "bis_connected_nums:%d", bis_connected_nums);
        if (!bis_connected_nums) {
            app_send_message(APP_MSG_LE_FIRST_DISCONNECT, 0);
        } else {
            app_send_message(APP_MSG_LE_SECOND_DISCONNECT, 0);
        }

#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_TX_EN)
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
        os_time_dly(2);
        app_broadcast_open();

        //关闭VM写RAM
        vm_ram_direct_2_flash_switch(TRUE);
        //如果开启了VM配置项暂存RAM功能则在断开连接后保存数据到vm_flash
        if (get_vm_ram_storage_enable()) {
            vm_flush2flash(0);
        }
#endif
        break;

    case BIG_EVENT_RECEIVER_CONNECT:
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_RX_EN)
        g_printf("BIG_EVENT_RECEIVER_CONNECT");
        //由于是异步操作需要加互斥量保护，避免broadcast_close的代码与其同时运行,添加的流程请放在互斥量保护区里面
        app_broadcast_mutex_pend(&mutex, __LINE__);

        wireless_mic_audio_status_reset();
        //关闭VM写RAM
        vm_ram_direct_2_flash_switch(FALSE);

        receiver_connected_status = 1;

        hdl = (big_hdl_t *)&event[1];

        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (app_big_hdl_info[i].used && (app_big_hdl_info[i].big_hdl == hdl->big_hdl)) {
                app_big_hdl_info[i].local_dev = WIRELESS_RX_DEV1;
                for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
                    if (!app_big_hdl_info[i].bis_hdl_info[j].bis_hdl)     {
                        app_big_hdl_info[i].bis_hdl_info[j].bis_hdl = hdl->bis_hdl[0];
                        app_big_hdl_info[i].bis_hdl_info[j].aux_hdl = hdl->aux_hdl;
                        app_big_hdl_info[i].bis_hdl_info[j].remote_dev = WIRELESS_TX_DEV1 << j;
                        memcpy(&app_big_hdl_info[i].bis_hdl_info[j].enc, &hdl->enc, sizeof(audio_param_t));
                        find = 1;
                        break;
                    }
                }
            }
            if (find) {
                break;
            }
        }

        if (!find) {
            //释放互斥量
            app_broadcast_mutex_post(&mutex, __LINE__);
            break;
        }

#if TCFG_AUTO_SHUT_DOWN_TIME
        sys_auto_shut_down_disable();
#endif

        g_printf("big_hdl:0x%x, bis_hdl:0x%x, aux_hdl:0x%x", hdl->big_hdl, hdl->bis_hdl[0], hdl->aux_hdl);

#if WIRELESS_MIC_PRODUCT_MODE
        bis_connected_nums++;
        ASSERT(bis_connected_nums <= BIG_MAX_BIS_NUMS && bis_connected_nums >= 0, "bis_connected_nums:%d", bis_connected_nums);
        if (bis_connected_nums == 1) {
            app_send_message(APP_MSG_LE_FIRST_CONNECT, 0);
        } else {
            app_send_message(APP_MSG_LE_SECOND_CONNECT, 0);
        }
#if (WIRELESS_MIC_BST_BIND_EN == 2) //0-不配对，1-TX生成配对码，2-RX生成配对码
        ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
        if ((ret <= 0) || (pair_code == 0x0)) {
            wireless_trans_get_pair_code("big_rx", (u8 *)&pair_code, 1);
            ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
            if (ret <= 0) {
                r_printf(">>>>>>wireless pair code save err, %d", __LINE__);
            }
        }
        wireless_custom_data_send_to_sibling('P', &pair_code, sizeof(u32), app_big_hdl_info[i].bis_hdl_info[j].remote_dev);
        wireless_trans_set_pair_code("big_rx", (u8 *)&pair_code);
#elif (WIRELESS_MIC_BST_BIND_EN == 1)  //由于TX事件比较快，导致TX发送配对码时，RX仍未准备好，此处改为引导TX发配对码
        pair_code = 0x55aa55aa;
        wireless_custom_data_send_to_sibling('P', &pair_code, sizeof(u32), app_big_hdl_info[i].bis_hdl_info[j].remote_dev);
#endif
#endif

        ret = broadcast_receiver_connect_deal((void *)hdl);
        if (ret < 0) {
            r_printf("broadcast_receiver_connect_deal fail");
        }

        //释放互斥量
        app_broadcast_mutex_post(&mutex, __LINE__);
#endif
        break;

    case BIG_EVENT_PERIODIC_DISCONNECT:
    case BIG_EVENT_RECEIVER_DISCONNECT:
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_RX_EN)
        g_printf("BIG_EVENT_RECEIVER_DISCONNECT");
        //由于是异步操作需要加互斥量保护，避免broadcast_close的代码与其同时运行,添加的流程请放在互斥量保护区里面
        app_broadcast_mutex_pend(&mutex, __LINE__);

        wireless_mic_audio_status_reset();
        receiver_connected_status = 0;

        hdl = (big_hdl_t *)&event[1];

        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (app_big_hdl_info[i].used && (app_big_hdl_info[i].big_hdl == hdl->big_hdl)) {
                for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
#if WIRELESS_MIC_PRODUCT_MODE
                    if (app_big_hdl_info[i].bis_hdl_info[j].bis_hdl == hdl->bis_hdl[0]) {
                        app_big_hdl_info[i].bis_hdl_info[j].bis_hdl = 0;
                        app_big_hdl_info[i].bis_hdl_info[j].aux_hdl = 0;
                        app_big_hdl_info[i].bis_hdl_info[j].remote_dev = 0;
                        find = 1;
                        break;
                    }
#else
                    app_big_hdl_info[i].bis_hdl_info[j].bis_hdl = 0;
                    app_big_hdl_info[i].bis_hdl_info[j].aux_hdl = 0;
                    app_big_hdl_info[i].bis_hdl_info[j].remote_dev = 0;
                    find = 1;
                    break;
#endif
                }
            }
            if (find) {
                break;
            }
        }

        if (!find) {
            //释放互斥量
            app_broadcast_mutex_post(&mutex, __LINE__);
            break;
        }

#if TCFG_AUTO_SHUT_DOWN_TIME
        sys_auto_shut_down_enable();   // 恢复自动关机
#endif

        g_printf("big_hdl:0x%x, bis_hdl:0x%x, aux_hdl:0x%x", hdl->big_hdl, hdl->bis_hdl[0], hdl->aux_hdl);

#if WIRELESS_MIC_PRODUCT_MODE
        bis_connected_nums--;
        ASSERT(bis_connected_nums <= BIG_MAX_BIS_NUMS && bis_connected_nums >= 0, "bis_connected_nums:%d", bis_connected_nums);
#endif

        if (!bis_connected_nums) {
            app_send_message(APP_MSG_LE_FIRST_DISCONNECT, 0);
        } else {
            app_send_message(APP_MSG_LE_SECOND_DISCONNECT, 0);
        }

        ret = broadcast_receiver_disconnect_deal((void *)hdl);
        if (ret < 0) {
            r_printf("broadcast_receiver_disconnect_deal fail");
        }

        if (!bis_connected_nums) {
            //关闭VM写RAM
            vm_ram_direct_2_flash_switch(TRUE);

            //如果开启了VM配置项暂存RAM功能则在关机前保存数据到vm_flash
            if (get_vm_ram_storage_enable() || get_vm_ram_storage_in_irq_enable()) {
                vm_flush2flash(0);
            }
        }

        //释放互斥量
        app_broadcast_mutex_post(&mutex, __LINE__);
#endif
        break;

    case BIG_EVENT_CUSTOM_DATA_SYNC:
        g_printf("BIG_EVENT_PADV_DATA_SYNC");
        broadcast_padv_data_deal((void *)&event[1]);
        break;

    default:
        result = -ESRCH;
        break;
    }

    return result;
}
APP_MSG_PROB_HANDLER(app_le_broadcast_msg_entry) = {
    .owner = 0xff,
    .from = MSG_FROM_BIG,
    .handler = app_broadcast_conn_status_event_handler,
};

/* --------------------------------------------------------------------------*/
/**
 * @brief 开启广播
 *
 * @return >=0:success
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_open()
{
    u8 i;
    u8 big_available_num = 0;
    int temp_broadcast_hdl = 0;
    big_parameter_t *params;

    if (!g_bt_hdl.init_ok || app_var.goto_poweroff_flag) {
        return -EPERM;
    }

    if (!app_broadcast_init_flag) {
        return -EPERM;
    }

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (!app_big_hdl_info[i].used) {
            big_available_num++;
        }
    }

    if (!big_available_num) {
        return -EPERM;
    }

    log_info("broadcast_open");

    app_broadcast_mutex_pend(&mutex, __LINE__);

#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_TX_EN)
    //初始化广播发送端参数
    params = set_big_params(WIRELESS_MIC_PRODUCT_MODE, BROADCAST_ROLE_TRANSMITTER, 0);

    //打开big，打开成功后会在函数app_broadcast_conn_status_event_handler做后续处理
    temp_broadcast_hdl = broadcast_transmitter(params);
#elif (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_RX_EN)
    //初始化广播接收端参数
    params = set_big_params(WIRELESS_MIC_PRODUCT_MODE, BROADCAST_ROLE_RECEIVER, 0);

    //打开big，打开成功后会在函数app_broadcast_conn_status_event_handler做后续处理
    temp_broadcast_hdl = broadcast_receiver(params);
#endif
    if (temp_broadcast_hdl >= 0) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (!app_big_hdl_info[i].used) {
                app_big_hdl_info[i].big_hdl = temp_broadcast_hdl;
                app_big_hdl_info[i].big_status = APP_BROADCAST_STATUS_START;
                app_big_hdl_info[i].used = 1;
                break;
            }
        }
    }

    app_broadcast_mutex_post(&mutex, __LINE__);
    return temp_broadcast_hdl;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭广播
 *
 * @param status:挂起还是停止
 *
 * @return 0:success
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_close(u8 status)
{
    u8 i;
    int ret = 0;

    if (!app_broadcast_init_flag) {
        return -EPERM;
    }

    log_info("broadcast_close");

    //由于是异步操作需要加互斥量保护，避免和开启开广播的流程同时运行,添加的流程请放在互斥量保护区里面
    app_broadcast_mutex_pend(&mutex, __LINE__);

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used && app_big_hdl_info[i].big_hdl) {
            ret = broadcast_close(app_big_hdl_info[i].big_hdl);
            memset(&app_big_hdl_info[i], 0, sizeof(struct app_big_hdl_t));
            app_big_hdl_info[i].big_status = status;
        }
    }

    receiver_connected_status = 0;
    bis_connected_nums = 0;

    app_send_message(APP_MSG_LE_FIRST_DISCONNECT, 0);

#if TCFG_AUTO_SHUT_DOWN_TIME
    sys_auto_shut_down_enable();   // 恢复自动关机
#endif

    //关闭VM写RAM
    vm_ram_direct_2_flash_switch(TRUE);

    //如果开启了VM配置项暂存RAM功能则在关机前保存数据到vm_flash
    if (get_vm_ram_storage_enable() || get_vm_ram_storage_in_irq_enable()) {
        vm_flush2flash(0);
    }

    //释放互斥量
    app_broadcast_mutex_post(&mutex, __LINE__);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播开关切换
 *
 * @return 0：操作成功
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_switch(void)
{
    u8 i;
    u8 find = 0;

    if (!g_bt_hdl.init_ok) {
        return -EPERM;
    }

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used) {
            find = 1;
            break;
        }
    }

    if (find) {
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    } else {
        app_broadcast_open();
    }

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化同步的状态数据的内容
 */
/* ----------------------------------------------------------------------------*/
static void app_broadcast_sync_data_init(void)
{
    app_broadcast_data_sync.volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    broadcast_sync_data_init(&app_broadcast_data_sync);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 更新广播同步状态的数据
 *
 * @param type:更新项
 * @param value:更新值
 *
 * @return 0:success
 */
/* ----------------------------------------------------------------------------*/
int update_broadcast_sync_data(u8 type, int value)
{
    u8 i;
    switch (type) {
    case BROADCAST_SYNC_VOL:
        if (app_broadcast_data_sync.volume == value) {
            return 0;
        }
        app_broadcast_data_sync.volume = value;
        break;
    default:
        return -ESRCH;
    }

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        broadcast_set_sync_data(app_big_hdl_info[i].big_hdl, &app_broadcast_data_sync, sizeof(struct broadcast_sync_info));
    }

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 接收到广播发送端的同步数据，并更新本地配置
 *
 * @param priv:接收到的同步数据
 *
 * @return
 */
/* ----------------------------------------------------------------------------*/
static int broadcast_padv_data_deal(void *priv)
{
    struct broadcast_sync_info *sync_data = (struct broadcast_sync_info *)priv;
    if (app_broadcast_data_sync.volume != sync_data->volume) {
        app_broadcast_data_sync.volume = sync_data->volume;
        app_audio_set_volume(APP_AUDIO_STATE_MUSIC, app_broadcast_data_sync.volume, 1);
        /* y_printf("----------> vol:%d\n", app_broadcast_data_sync.volume); */
        app_send_message(APP_MSG_VOL_CHANGED, app_broadcast_data_sync.volume);
    }
    memcpy(&app_broadcast_data_sync, sync_data, sizeof(struct broadcast_sync_info));
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播资源初始化
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_init(void)
{
    if (!g_bt_hdl.init_ok) {
        return;
    }

    if (app_broadcast_init_flag) {
        return;
    }

    int os_ret = os_mutex_create(&mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    broadcast_init();
    app_broadcast_sync_data_init();
    app_broadcast_init_flag = 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播资源卸载
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_uninit(void)
{
    if (!g_bt_hdl.init_ok) {
        return;
    }

    if (!app_broadcast_init_flag) {
        return;
    }

    app_broadcast_init_flag = 0;

    int os_ret = os_mutex_del(&mutex, OS_DEL_NO_PEND);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
    }

    broadcast_uninit();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播接收端配对事件回调
 *
 * @param event：配对事件
 * @param priv：事件附带的参数
 */
/* ----------------------------------------------------------------------------*/
static void broadcast_pair_rx_event_callback(const PAIR_EVENT event, void *priv)
{
    switch (event) {
    case PAIR_EVENT_RX_PRI_CHANNEL_CREATE_SUCCESS:
        u32 *private_connect_access_addr = (u32 *)priv;
        g_printf("PAIR_EVENT_RX_PRI_CHANNEL_CREATE_SUCCESS:0x%x", *private_connect_access_addr);
        int ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, private_connect_access_addr, sizeof(u32));
        if (ret <= 0) {
            r_printf(">>>>>>wireless pair code save err");
        }
        break;

    case PAIR_EVENT_RX_OPEN_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_RX_OPEN_PAIR_MODE_SUCCESS");
        break;

    case PAIR_EVENT_RX_CLOSE_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_RX_CLOSE_PAIR_MODE_SUCCESS");
        app_broadcast_open();
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播发送端配对事件回调
 *
 * @param event：配对事件
 * @param priv：事件附带的参数
 */
/* ----------------------------------------------------------------------------*/
static void broadcast_pair_tx_event_callback(const PAIR_EVENT event, void *priv)
{
    switch (event) {
    case PAIR_EVENT_TX_PRI_CHANNEL_CREATE_SUCCESS:
        u32 *private_connect_access_addr = (u32 *)priv;
        g_printf("PAIR_EVENT_TX_PRI_CHANNEL_CREATE_SUCCESS:0x%x", *private_connect_access_addr);
        int ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, private_connect_access_addr, sizeof(u32));
        if (ret <= 0) {
            r_printf(">>>>>>wireless pair code save err");
        }
        break;

    case PAIR_EVENT_TX_OPEN_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_TX_OPEN_PAIR_MODE_SUCCESS");
        break;

    case PAIR_EVENT_TX_CLOSE_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_TX_CLOSE_PAIR_MODE_SUCCESS");
        app_broadcast_open();
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播进入配对模式
 *
 * @param role：广播角色接收端还是发送端
 * @param mode：0-广播配对，1-连接配对
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_enter_pair(u8 role, u8 mode)
{
    int ret;
    u32 private_connect_access_addr = 0;

    app_broadcast_close(APP_BROADCAST_STATUS_STOP);

    ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &private_connect_access_addr, sizeof(u32));
    if (role == BROADCAST_ROLE_UNKNOW) {
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_TX_EN)
        broadcast_enter_pair(BROADCAST_ROLE_TRANSMITTER, mode, (void *)&pair_tx_cb, private_connect_access_addr);
#elif (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_RX_EN)
        broadcast_enter_pair(BROADCAST_ROLE_RECEIVER, mode, (void *)&pair_rx_cb, private_connect_access_addr);
#endif
    } else if (role == BROADCAST_ROLE_TRANSMITTER) {
        broadcast_enter_pair(role, mode, (void *)&pair_tx_cb, private_connect_access_addr);
    } else if (role == BROADCAST_ROLE_RECEIVER) {
        broadcast_enter_pair(role, mode, (void *)&pair_rx_cb, private_connect_access_addr);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 退出广播配对模式
 *
 * @param role：广播角色接收端还是发送端
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_exit_pair(u8 role)
{
    if (role == BROADCAST_ROLE_UNKNOW) {
#if (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_TX_EN)
        broadcast_exit_pair(BROADCAST_ROLE_TRANSMITTER);
#elif (TCFG_LE_AUDIO_APP_CONFIG & LE_AUDIO_JL_BIS_RX_EN)
        broadcast_exit_pair(BROADCAST_ROLE_RECEIVER);
#endif
    } else {
        broadcast_exit_pair(role);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 清楚配对信息
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_remove_pair(void)
{
    u32 private_connect_access_addr = 0;
    app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    int ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &private_connect_access_addr, sizeof(u32));
    if (!ret) {
        log_error(RedBoldBlink">>>>>>wireless pair code erase err"Reset);
    }
    app_broadcast_open();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 非蓝牙后台情况下，在其他音源模式开启BIG，前提要先开蓝牙协议栈
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_open_in_other_mode()
{
    app_broadcast_init();
    app_broadcast_open();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 非蓝牙后台情况下，在其他音源模式关闭BIG
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_close_in_other_mode()
{
    app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    app_broadcast_uninit();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 自定义数据发送接口
 *
 * @param device:发送给远端设备的标识
 * @param data:需要发送的数据
 * @param length:数据长度
 *
 * @return slen:实际发送数据的总长度
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_send_custom_data(u8 device, void *data, size_t length)
{
    u8 i, j;
    int slen = 0;
    struct wireless_data_callback_func *p;

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used) {
            for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
                if (app_big_hdl_info[i].bis_hdl_info[j].remote_dev & device) {
                    slen += broadcast_send_custom_data(app_big_hdl_info[i].bis_hdl_info[j].aux_hdl, data, length);
                }
            }
        }
    }

    if (slen) {
        list_for_each_wireless_data_callback(p) {
            if (p->tx_events_suss) {
                if (p->tx_events_suss(data, length)) {
                    break;
                }
            }
        }
    }

    return slen;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取bis连接数量
 *
 * @return bis连接数量
 */
/* ----------------------------------------------------------------------------*/
u8 get_bis_connected_num(void)
{
    return bis_connected_nums;
}

#if WIRELESS_MIC_BST_BIND_EN
static void receive_remote_pcode_handler(u16 acl_hdl, u8 arg_num, int *argv)
{
    u8 i, j;
    int trans_role = argv[0];
    u8 *data = (u8 *)argv[1];
    int len = argv[2];
    u32 pair_code = 0;
    int ret = 0;

    if (get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
                if (app_big_hdl_info[i].bis_hdl_info[j].aux_hdl == acl_hdl) {
                    memcpy(&pair_code, data, sizeof(u32));
                    if (pair_code == 0x55aa55aa) {  //RX引导数据，TX发送配对码给RX
#if (WIRELESS_MIC_BST_BIND_EN == 1)
                        ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
                        if ((ret <= 0) || (pair_code == 0)) {
                            wireless_trans_get_pair_code("big_tx", (u8 *)&pair_code, 1);
                            ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
                            if (ret <= 0) {
                                log_error(RedBoldBlink"wireless pair code save err, %d"Reset, __LINE__);
                            } else {
                                log_info(GreenBold"receive_remote_pcode_handler pair_code: 0x%x"Reset, pair_code);
                            }
                        }
                        wireless_custom_data_send_to_sibling('P', &pair_code, sizeof(u32), app_big_hdl_info[i].bis_hdl_info[j].remote_dev);
                        wireless_trans_set_pair_code("big_tx", (u8 *)&pair_code);
#endif
                    } else {
                        wireless_trans_set_pair_code("big_tx", (u8 *)&pair_code);
                        ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
                        if (ret <= 0) {
                            log_error(RedBoldBlink"wireless pair code save err, %d"Reset, __LINE__);
                        } else {
                            log_info(GreenBold"receive_remote_pcode_handler pair_code: 0x%x"Reset, pair_code);
                        }
                    }
                    break;
                }
            }
        }
    } else if (get_broadcast_role() == BROADCAST_ROLE_RECEIVER) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            for (j = 0; j < BIG_MAX_BIS_NUMS; j++) {
                if (app_big_hdl_info[i].bis_hdl_info[j].aux_hdl == acl_hdl) {
                    int ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
                    if ((ret <= 0) || (pair_code == 0)) {
                        memcpy(&pair_code, data, sizeof(u32));
                        wireless_trans_set_pair_code("big_rx", (u8 *)&pair_code);
                        ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
                        if (ret <= 0) {
                            log_error(RedBoldBlink"wireless pair code save err, %d"Reset, __LINE__);
                        } else {
                            log_info(GreenBold"receive_remote_pcode_handler pair_code: 0x%x"Reset, pair_code);
                        }
                    } else {
                        log_info(GreenBold"receive_remote_pcode_handler pair_code: 0x%x"Reset, pair_code);
                        wireless_custom_data_send_to_sibling('P', &pair_code, sizeof(u32), app_big_hdl_info[i].bis_hdl_info[j].remote_dev);
                        wireless_trans_set_pair_code("big_rx", (u8 *)&pair_code);
                    }
                    break;
                }
            }
        }
    }
    //由于转发流程中申请了内存，因此执行完毕后必须释放
    if (data) {
        free(data);
    }
}

WIRELESS_CUSTOM_DATA_STUB_REGISTER(pcode_bind) = {
    .uuid = 'P',
    .task_name = "app_core",
    .func = receive_remote_pcode_handler,
};
#endif

#endif

