/*************************************************************************************************/
/*!
*  \file      audio_mode.c
*
*  \brief   用于处于无线传输的模式设置和参数
*
*  Copyright (c) 2011-2022 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "app_config.h"
#include "live_audio.h"
#include "wireless_params.h"
#include "audio_mode.h"


#if (TCFG_CONNECTED_ENABLE && CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)

#if ((defined TCFG_LIVE_AUDIO_LOW_LATENCY_EN) && TCFG_LIVE_AUDIO_LOW_LATENCY_EN)
#define CAPTURE_DELAY_TIME      1 + (JLA_CODING_CHANNEL / 2) + 7
#else

#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == ROLE_CENTRAL))
#define CAPTURE_DELAY_TIME      (5 + 10 + 5) /*sample 5ms, tx_align->rx 8ms*/
#else
#define CAPTURE_DELAY_TIME      (5 + 10) /*sample 5ms, tx_align->rx 8ms*/
#endif /*defined(CIG_MULTIPLE_CAPTURE_EN) && (CIG_MULTIPLE_CAPTURE_EN)*/

#endif /*((defined TCFG_LIVE_AUDIO_LOW_LATENCY_EN) && TCFG_LIVE_AUDIO_LOW_LATENCY_EN)*/

#else

#if ((defined TCFG_LIVE_AUDIO_LOW_LATENCY_EN) && TCFG_LIVE_AUDIO_LOW_LATENCY_EN)
#define CAPTURE_DELAY_TIME      4 + (JLA_CODING_CHANNEL / 2)
#else
#define CAPTURE_DELAY_TIME      10
#endif

#endif /*TCFG_CONNECTED_ENABLE && CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX*/
struct live_audio_mode_context {
    u8 mode;
    u32 source_type;
    u32 sample_rate;
    struct live_audio_mode_ops *ops;
};

static struct live_audio_mode_context g_live_audio_mode[LIVE_AUDIO_CAPTURE_MAX_MODE] = {0};

extern int CONFIG_A2DP_DELAY_TIME;

#if 0
extern void a2dp_dec_close();
static int live_a2dp_play_open(struct audio_path *path)
{
    //TODO
    return NULL;
}

static int live_a2dp_play_close(void *player)
{
    return a2dp_dec_close();
}

const struct live_audio_mode_ops audio_mode_ops[] = {
    {
        .capture_open = live_aux_capture_open,
        .capture_close = live_aux_capture_close,
        .capture_start = live_aux_capture_start,
        .capture_suspend = live_aux_capture_stop,

        .play_open = live_aux_play_open,
        .play_close = live_aux_play_close,
    },

    {
        .capture_open = live_a2dp_capture_open,
        .capture_close = live_a2dp_capture_close,
        .capture_start = live_a2dp_capture_start,
        .capture_suspend = live_a2dp_capture_suspend,

        .play_open = live_a2dp_play_open,
        .play_close = live_a2dp_play_close,
    },
};
#endif

int live_audio_mode_delay_time(u8 mode)
{
    int delay_time = CAPTURE_DELAY_TIME;

    switch (mode) {
    case LIVE_A2DP_CAPTURE_MODE:
        delay_time = CONFIG_A2DP_DELAY_TIME;
        break;
    case LIVE_FILE_CAPTURE_MODE:
        delay_time = 30;
        break;
    case LIVE_IIS_CAPTURE_MODE:
#if (TCFG_CONNECTED_ENABLE && CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)
        delay_time = CAPTURE_DELAY_TIME;
#else
        delay_time = 4 + 6;
#endif

        break;
    }

    return delay_time;
}

int live_audio_mode_setup(u8 mode, u32 sample_rate, struct live_audio_mode_ops *ops)
{
    g_live_audio_mode[mode].mode = mode;
    g_live_audio_mode[mode].sample_rate = sample_rate;
    g_live_audio_mode[mode].ops = ops;

    return 0;
}

int live_audio_mode_play_stop(u8 mode)
{
    if (g_live_audio_mode[mode].ops) {
        g_live_audio_mode[mode].ops->play_close(NULL);
    }

    return 0;
}

int live_audio_mode_play_start(u8 mode)
{
    if (g_live_audio_mode[mode].ops) {
        g_live_audio_mode[mode].ops->play_open(NULL);
    }

    return 0;
}

int live_audio_mode_play_status(u8 mode)
{
    if (g_live_audio_mode[mode].ops &&
        g_live_audio_mode[mode].ops->play_status) {
        return g_live_audio_mode[mode].ops->play_status();
    }

    return 0;
}

int live_audio_mode_get_capture_params(u8 mode, struct audio_path *path)
{
    memset(path, 0x0, sizeof(struct audio_path));
    path->fmt.coding_type = AUDIO_CODING_PCM;
    path->fmt.sample_rate = g_live_audio_mode[mode].sample_rate;
    path->fmt.channel = JLA_CODING_CHANNEL;
    path->fmt.priv = (void *)g_live_audio_mode[mode].source_type;
    path->delay_time = 0;
    path->input.path = (void *)g_live_audio_mode[mode].ops;//&audio_mode_ops[g_live_audio_mode];
    return 0;
}

int live_audio_mode_get_broadcast_params(u8 mode, struct audio_path *path)
{
    memset(path, 0x0, sizeof(struct audio_path));

    path->delay_time = live_audio_mode_delay_time(mode) + get_big_sdu_period_ms() + get_big_mtl_time();
    path->fmt.coding_type = AUDIO_CODING_JLA;
    path->fmt.channel = JLA_CODING_CHANNEL;
    path->fmt.sample_rate = JLA_CODING_SAMPLERATE;
    path->fmt.frame_len = JLA_CODING_FRAME_LEN;
    path->fmt.bit_rate = JLA_CODING_BIT_RATE;
    return 0;
}

int live_audio_mode_get_cis_params(u8 mode, struct audio_path *path)
{
    memset(path, 0x0, sizeof(struct audio_path));
    printf("live audio cis delay : %d, %d, %d\n", live_audio_mode_delay_time(mode), get_cig_sdu_period_ms(), get_cig_mtl_time());

    path->delay_time = live_audio_mode_delay_time(mode) + get_cig_sdu_period_ms() + get_cig_mtl_time();
    path->fmt.coding_type = AUDIO_CODING_JLA;
    path->fmt.channel = JLA_CODING_CHANNEL;
    path->fmt.sample_rate = JLA_CODING_SAMPLERATE;
    path->fmt.frame_len = JLA_CODING_FRAME_LEN;
    path->fmt.bit_rate = JLA_CODING_BIT_RATE;
    return 0;
}
