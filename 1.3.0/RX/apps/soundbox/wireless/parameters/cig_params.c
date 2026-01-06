/*********************************************************************************************
    *   Filename        : cig_params.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:31

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "app_task.h"
#include "audio_base.h"
#include "connected_api.h"
#include "wireless_params.h"
#include "live_audio.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/
/*! \brief 各个模式发包间隔 */
#define BT_SDU_PERIOD_MS        10
#define MUSIC_SDU_PERIOD_MS     10
#define AUX_SDU_PERIOD_MS       10
#define MIC_SDU_PERIOD_MS       10
#define IIS_SDU_PERIOD_MS       10

/*! \brief 配对名 */
#define CIG_PAIR_NAME           "br28_cig_soundbox"

/*! \breief 配置连接设备数，最大配置为2  */
#define USED_CIS_NUM            LEA_CIG_CONNECTION_NUM

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static struct jla_codec_params cig_bt_codec_params = {
    .sdu_period_ms = BT_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUND_A2DP,
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
};

static struct jla_codec_params cig_music_codec_params = {
    .sdu_period_ms = MUSIC_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUND_FILE,
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
};

static struct jla_codec_params cig_aux_codec_params = {
    .sdu_period_ms = AUX_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUDN_AUX,
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
};

static struct jla_codec_params cig_mic_codec_params = {
    .sdu_period_ms = MIC_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUND_MIC,
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
};

static struct jla_codec_params cig_iis_codec_params = {
#if (TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS)
    .sdu_period_ms = IIS_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUND_IIS,
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
#endif
};

static struct jla_codec_params cig_receiver_codec_params = {
    .nch = JLA_CODING_CHANNEL,
    .coding_type = AUDIO_CODING_JLA,
    .sample_rate = JLA_CODING_SAMPLERATE,
    .frame_size = JLA_CODING_FRAME_LEN,
    .bit_rate = JLA_CODING_BIT_RATE,
};

static cig_parameter_t cig_bt_tx_param = {
    .pair_name      = CIG_PAIR_NAME,
    .cb             = &cig_central_cb,
    .pair_en        = 1,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = 2,
        .rtnPToC    = 2,
    },

    .cis[1] = {
        .rtnCToP    = 1,
        .rtnPToC    = 1,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_music_tx_param = {
    .pair_name      = CIG_PAIR_NAME,
    .cb             = &cig_central_cb,
    .pair_en        = 1,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = 3,
        .rtnPToC    = 3,
    },

    .cis[1] = {
        .rtnCToP    = 1,
        .rtnPToC    = 1,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_aux_tx_param = {
    .pair_name      = CIG_PAIR_NAME,
    .cb             = &cig_central_cb,
    .pair_en        = 1,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = 2,
        .rtnPToC    = 2,
    },

    .cis[1] = {
        .rtnCToP    = 2,
        .rtnPToC    = 2,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_mic_tx_param = {
    .pair_name      = CIG_PAIR_NAME,
    .cb             = &cig_central_cb,
#if WIRELESS_PAIR_BONDING
    .pair_en        = 1,
#else
    .pair_en        = 0,
#endif
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = 3,
        .rtnPToC    = 3,
    },

    .cis[1] = {
        .rtnCToP    = 1,
        .rtnPToC    = 1,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_iis_tx_param = {
    .pair_name      = CIG_PAIR_NAME,
    .cb             = &cig_central_cb,
    .pair_en        = 1,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = 3,
        .rtnPToC    = 3,
    },

    .cis[1] = {
        .rtnCToP    = 1,
        .rtnPToC    = 1,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_rx_param = {
    .pair_name = CIG_PAIR_NAME,
    .cb        = &cig_perip_cb,
#if WIRELESS_PAIR_BONDING
    .pair_en        = 1,
#else
    .pair_en        = 0,
#endif

    .vdr = {
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};


static cig_parameter_t *central_params = NULL;
static cig_parameter_t *perip_params = NULL;
static struct jla_codec_params *codec_params = NULL;
static u16 cig_transmit_data_len = 0;
static u32 enc_output_frame_len = 0;
static u32 dec_input_buf_len = 0;
static u32 enc_output_buf_len = 0;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*! \brief 每包编码数据长度 */
/* (int)((JLA_CODING_FRAME_LEN / 10) * (JLA_CODING_BIT_RATE / 1000 / 8) + 2) */
/* 如果码率超过96K,即帧长超过122,就需要将每次传输数据大小 修改为一帧编码长度 */
static u32 calcul_cig_enc_output_frame_len(u16 frame_len, u32 bit_rate)
{
    return (frame_len * bit_rate / 1000 / 8 / 10 + 2);
}

u32 get_cig_enc_output_frame_len(void)
{
    ASSERT(enc_output_frame_len, "enc_output_frame_len is 0");
    return enc_output_frame_len;
}

static u16 calcul_cig_transmit_data_len(u32 encode_output_frame_len, u16 period, u16 codec_frame_len)
{
    return (encode_output_frame_len * (period * 10 / codec_frame_len));
}

u16 get_cig_transmit_data_len(void)
{
    ASSERT(cig_transmit_data_len, "cig_transmit_data_len is 0");
    return cig_transmit_data_len;
}

static u32 calcul_cig_enc_output_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 2);
}

u32 get_cig_enc_output_buf_len(void)
{
    ASSERT(enc_output_buf_len, "enc_output_buf_len is 0");
    return enc_output_buf_len;
}

static u32 calcul_cig_dec_input_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 10);
}

u32 get_cig_dec_input_buf_len(void)
{
    ASSERT(dec_input_buf_len, "dec_input_buf_len is 0");
    return dec_input_buf_len;
}

u32 get_cig_sdu_period_ms(void)
{
    return codec_params->sdu_period_ms;
}

struct jla_codec_params *get_cig_codec_params_hdl(void)
{
    return codec_params;
}

u32 get_cig_mtl_time(void)
{
    return 0;//central_params->mtlCToP;
}

void set_cig_hdl(u8 role, u8 cig_hdl)
{
    if ((role == CONNECTED_ROLE_CENTRAL) && central_params) {
        central_params->cig_hdl = cig_hdl;
    } else if ((role == CONNECTED_ROLE_PERIP) && perip_params) {
        perip_params->cig_hdl = cig_hdl;
    }
}

cig_parameter_t *set_cig_params(u8 app_task, u8 role, u8 pair_without_addr)
{
    int ret;
    u64 pair_addr;
    if (role == CONNECTED_ROLE_CENTRAL) {
        switch (app_task) {
        case APP_BT_TASK:
            central_params = &cig_bt_tx_param;
            codec_params = &cig_bt_codec_params;
            break;
        case APP_MUSIC_TASK:
            central_params = &cig_music_tx_param;
            codec_params = &cig_music_codec_params;
            break;
        case APP_LINEIN_TASK:
            central_params = &cig_aux_tx_param;
            codec_params = &cig_aux_codec_params;
            break;
        case APP_LIVE_MIC_TASK:
            central_params = &cig_mic_tx_param;
            codec_params = &cig_mic_codec_params;
            break;
        case APP_LIVE_IIS_TASK:
            central_params = &cig_iis_tx_param;
            codec_params = &cig_iis_codec_params;
            break;
        default:
            central_params = NULL;
            codec_params = NULL;
            break;
        }
#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
        codec_params->nch = 1;
        codec_params->bit_rate /= 2;
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, JLA_CODING_BIT_RATE);
#else
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
#endif
        cig_transmit_data_len = calcul_cig_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_ms, codec_params->frame_size);
        dec_input_buf_len = calcul_cig_dec_input_buf_len(cig_transmit_data_len);
        enc_output_buf_len = calcul_cig_enc_output_buf_len(cig_transmit_data_len);
        if (central_params) {

#if (CIG_TRANSPORT_MODE == CIG_MODE_CToP)
            central_params->mtlCToP = codec_params->sdu_period_ms;
            central_params->sduIntUsCToP = codec_params->sdu_period_ms * 1000L;
            central_params->cis[0].maxSduCToP = cig_transmit_data_len;
            central_params->cis[1].maxSduCToP = cig_transmit_data_len;
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_CToP)

#if (CIG_TRANSPORT_MODE == CIG_MODE_PToC)
            central_params->mtlPToC = codec_params->sdu_period_ms;
            central_params->sduIntUsPToC = codec_params->sdu_period_ms * 1000L;
            central_params->cis[0].maxSduPToC = cig_transmit_data_len;
            central_params->cis[1].maxSduPToC = cig_transmit_data_len;
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_PToC)

#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)
            central_params->mtlCToP = codec_params->sdu_period_ms;
            central_params->mtlPToC = codec_params->sdu_period_ms;
            central_params->sduIntUsCToP = codec_params->sdu_period_ms * 1000L;
            central_params->sduIntUsPToC = codec_params->sdu_period_ms * 1000L;
            central_params->cis[0].maxSduCToP = cig_transmit_data_len;
            central_params->cis[0].maxSduPToC = cig_transmit_data_len;
            central_params->cis[1].maxSduCToP = cig_transmit_data_len;
            central_params->cis[1].maxSduPToC = cig_transmit_data_len;
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)

        }
        if (pair_without_addr) {
            central_params->cis[0].pri_ch = 0;
            central_params->cis[1].pri_ch = 0;
        } else {
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            central_params->cis[0].pri_ch = pair_addr;
            g_printf("cis0.pri_ch:");
            put_buf(&central_params->cis[0].pri_ch, sizeof(central_params->cis[0].pri_ch));
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE1, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            central_params->cis[1].pri_ch = pair_addr;
            g_printf("cis1.pri_ch:");
            put_buf(&central_params->cis[1].pri_ch, sizeof(central_params->cis[1].pri_ch));
        }
        return central_params;
    }

    if (role == CONNECTED_ROLE_PERIP) {
        perip_params = &cig_rx_param;
        codec_params = &cig_receiver_codec_params;
        if (app_task == APP_BT_TASK) {
            codec_params->sdu_period_ms = BT_SDU_PERIOD_MS;
        } else if (app_task == APP_MUSIC_TASK) {
            codec_params->sdu_period_ms = MUSIC_SDU_PERIOD_MS;
        } else if (app_task == APP_LINEIN_TASK) {
            codec_params->sdu_period_ms = AUX_SDU_PERIOD_MS;
        } else if (app_task == APP_LIVE_MIC_TASK) {
            codec_params->sdu_period_ms = MIC_SDU_PERIOD_MS;
        } else if (app_task == APP_LIVE_IIS_TASK) {
            codec_params->sdu_period_ms = IIS_SDU_PERIOD_MS;
        }
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
        cig_transmit_data_len = calcul_cig_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_ms, codec_params->frame_size);
        dec_input_buf_len = calcul_cig_dec_input_buf_len(cig_transmit_data_len);
        if (pair_without_addr) {
            perip_params->perip.pri_ch = 0;
        } else {
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            perip_params->perip.pri_ch = pair_addr;
            g_printf("perip.pri_ch:");
            put_buf(&perip_params->perip.pri_ch, sizeof(perip_params->perip.pri_ch));
        }
        return perip_params;
    }

    return NULL;
}

