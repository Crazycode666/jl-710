/*********************************************************************************************
    *   Filename        : le_audio_common.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2023-8-18 19:09

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "app_main.h"
#include "audio_base.h"
#include "wireless_trans.h"
#include "le_audio_stream.h"
#include "le_audio_player.h"
#include "le_broadcast.h"
#include "le_connected.h"
#include "app_le_broadcast.h"
#include "app_le_connected.h"

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN)) || \
    (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))

/**************************************************************************************************
  Static Prototypes
 **************************************************************************************************/

/**************************************************************************************************
  Extern Global Variables
**************************************************************************************************/
extern const struct le_audio_mode_ops le_audio_wireless_mic_ops;

/**************************************************************************************************
  Local Global Variables
**************************************************************************************************/
static u8 lea_product_test_name[16];
static u8 le_audio_pair_name[16];
static struct le_audio_mode_ops *broadcast_audio_switch_ops = NULL; /*!< le audio和local audio切换回调接口指针 */
static struct le_audio_mode_ops *connected_audio_switch_ops = NULL; /*!< le audio和local audio切换回调接口指针 */

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
void read_le_audio_product_name(void)
{
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN)) || \
    (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    int len = syscfg_read(CFG_LEA_PRODUCET_TEST_NAME, lea_product_test_name, sizeof(lea_product_test_name));
    if (len <= 0) {
        r_printf("ERR:Can not read the product test name\n");
        return;
    }

    put_buf((const u8 *)lea_product_test_name, sizeof(lea_product_test_name));
    r_printf("product_test_name:%s", lea_product_test_name);
#endif
}

void read_le_audio_pair_name(void)
{
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN)) || \
    (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    int len = syscfg_read(CFG_LEA_PAIR_NAME, le_audio_pair_name, sizeof(le_audio_pair_name));
    if (len <= 0) {
        r_printf("ERR:Can not read the le audio pair name\n");
        return;
    }

    put_buf((const u8 *)le_audio_pair_name, sizeof(le_audio_pair_name));
    y_printf("pair_name:%s", le_audio_pair_name);
#endif
}

const char *get_le_audio_product_name(void)
{
    return (const char *)lea_product_test_name;
}

const char *get_le_audio_pair_name(void)
{
    return (const char *)le_audio_pair_name;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 注册le audio和local audio切换回调接口
 *
 * @param ops:le audio和local audio切换回调接口结构体
 */
/* ----------------------------------------------------------------------------*/
static void le_audio_switch_ops_callback(void *ops)
{
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    broadcast_audio_switch_ops = (struct le_audio_mode_ops *)ops;
#elif (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    connected_audio_switch_ops = (struct le_audio_mode_ops *)ops;
#endif
}

struct le_audio_mode_ops *get_broadcast_audio_sw_ops()
{
    return broadcast_audio_switch_ops;
}

struct le_audio_mode_ops *get_connected_audio_sw_ops()
{
    return connected_audio_switch_ops;
}

int le_audio_ops_register(u8 mode)
{
    g_printf("le_audio_ops_register:%d", mode);

    switch (mode) {
#if TCFG_AUDIO_MIC_ENABLE
    case APP_MODE_BT:
        le_audio_switch_ops_callback((void *)&le_audio_wireless_mic_ops);
        break;
#endif
    default:
        break;
    }
    return 0;
}

int le_audio_ops_unregister(void)
{
    le_audio_switch_ops_callback(NULL);
    return 0;
}

#endif

int le_audio_scene_deal(int scene)
{
    return -EPERM;
}

u8 get_le_audio_curr_role() //1:transmitter; 2:recevier
{
#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    return get_broadcast_role();
#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
#if  (LEA_CIG_TRANS_MODE == LEA_TRANS_DUPLEX)
    return 1;
#else
    return get_connected_role();
#endif
#endif

    return 0;
}

