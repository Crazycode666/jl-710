#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".bt_event_func.data.bss")
#pragma data_seg(".bt_event_func.data")
#pragma const_seg(".bt_event_func.text.const")
#pragma code_seg(".bt_event_func.text")
#endif
#include "app_main.h"
#include "clock_manager/clock_manager.h"
#include "app_le_broadcast.h"
#include "app_le_connected.h"
#include "wireless_trans.h"
#include "bt_event_func.h"

/*************************************************************************************************/
/*!
 *  \brief      蓝牙初始化完成
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_status_init_ok(void)
{
    g_bt_hdl.init_ok = 1;

#if TCFG_NORMAL_SET_DUT_MODE
#if TCFG_USER_BLE_ENABLE
    printf("ble set dut mode\n");
    extern void ble_standard_dut_test_init(void);
    ble_standard_dut_test_init();
    return;
#endif
#endif

#if (CONFIG_BT_MODE == BT_FCC)
    return;
#endif

#if ((CONFIG_BT_MODE == BT_BQB)||(CONFIG_BT_MODE == BT_PER))
    extern void ble_standard_dut_test_init(void);
    ble_standard_dut_test_init();
    return;
#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_BIS_TX_EN | LE_AUDIO_JL_BIS_RX_EN))
    le_audio_ops_register(APP_MODE_BT);
    app_broadcast_init();
    app_broadcast_open();
#endif

#if (TCFG_LE_AUDIO_APP_CONFIG & (LE_AUDIO_JL_CIS_CENTRAL_EN | LE_AUDIO_JL_CIS_PERIPHERAL_EN))
    le_audio_ops_register(APP_MODE_BT);
    app_connected_init();
    app_connected_open(0);
#endif
}

