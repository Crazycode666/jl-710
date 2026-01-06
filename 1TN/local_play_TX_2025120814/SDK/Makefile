
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := pi32v2-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/pi32v2-lib/r3
SYS_INC_DIR := C:/JL/pi32/pi32v2-include
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := tools/utils/fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := cpu/br56/tools/download.bat
RUN_POST_SCRIPT := cpu\br56\tools\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32v2/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib/r3
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := cpu/br56/tools/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := cpu/br56/tools/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs

# 编译参数设置
CFLAGS := \
	-target pi32v2 \
	-mcpu=r3 \
	-integrated-as \
	-flto \
	-Wuninitialized \
	-Wno-invalid-noreturn \
	-fno-common \
	-integrated-as \
	-Oz \
	-g \
	-flto \
	-fallow-pointer-null \
	-fprefer-gnu-section \
	-Wno-shift-negative-value \
	-fms-extensions \
	-fdiscrete-bitfield-abi \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-DCONFIG_RELEASE_ENABLE \
	-DCONFIG_WIRELESS_MIC_CASE_ENABLE \
	-DCONFIG_CPU_BR56 \
	-DCONFIG_IIC_VERSION2 \
	-DBR56_FPGA_CODE_VERIFY \
	-DCONFIG_SUPPORT_AES_CCM_FOR_EDR \
	-DCONFIG_AES_CCM_RXMAXBUF_EXT_MICLEN \
	-DCONFIG_NEW_BREDR_ENABLE \
	-DCONFIG_NEW_MODEM_ENABLE \
	-DCONFIG_NEW_TWS_FORWARD_ENABLE \
	-DCONFIG_UCOS_ENABLE \
	-DCONFIG_EQ_SUPPORT_ASYNC \
	-DEQ_CORE_V1 \
	-DCONFIG_BT_CTRLER_IN_MASK \
	-DCONFIG_AAC_CODEC_FFT_USE_MUTEX \
	-DCONFIG_SIN_TONE_V2 \
	-DCONFIG_DNS_ENABLE \
	-DCONFIG_DMS_MALLOC \
	-DCONFIG_MMU_ENABLE \
	-DCONFIG_CBUF_IN_MASKROM \
	-DCONFIG_SBC_CODEC_HW \
	-DCONFIG_MSBC_CODEC_HW \
	-DCONFIG_AEC_M=256 \
	-DCONFIG_AUDIO_ONCHIP \
	-DCONFIG_MEDIA_NEW_ENABLE \
	-DCONFIG_SUPPORT_EX_TWS_ADJUST \
	-D__GCC_PI32V2__ \
	-DCONFIG_NEW_ECC_ENABLE \
	-DCONFIG_CODE_MOVABLE_ENABLE \
	-DCONFIG_RESFILE_SUPPORT_MULT_DEVICE=0 \
	-DCONFIG_256K_FLASH \
	-DTCFG_USER_TWS_ENABLE=0 \
	-DCONFIG_EFFECT_CORE_V2_ENABLE \
	-DCONFIG_OS_AFFINITY_ENABLE=0 \
	-DEVENT_HANDLER_NUM_CONFIG=2 \
	-DEVENT_TOUCH_ENABLE_CONFIG=0 \
	-DEVENT_POOL_SIZE_CONFIG=256 \
	-DCONFIG_EVENT_KEY_MAP_ENABLE=0 \
	-DTIMER_POOL_NUM_CONFIG=5 \
	-DAPP_ASYNC_POOL_NUM_CONFIG=0 \
	-DSDFILE_STORAGE=0 \
	-DVFS_FILE_POOL_NUM_CONFIG=1 \
	-DFS_VERSION=0x020001 \
	-DFATFS_VERSION=0x020101 \
	-DSDFILE_VERSION=0x020000 \
	-DVFS_ENABLE=0 \
	-DVM_MAX_SIZE_CONFIG=16*1024 \
	-DVM_ITEM_MAX_NUM=256 \
	-DCONFIG_NEW_CFG_TOOL_ENABLE \
	-DCONFIG_LITE_AEC_ENABLE=1 \
	-DAUDIO_REC_LITE \
	-DAUDIO_DEC_LITE \
	-DAUDIO_REC_POOL_NUM=1 \
	-DAUDIO_DEC_POOL_NUM=3 \
	-DCONFIG_BTCTRLER_TASK_DEL_ENABLE \
	-DCONFIG_LINK_DISTURB_SCAN_ENABLE=0 \
	-DCONFIG_ITEM_FORMAT_VM \
	-DCONFIG_WIRELESS_MIC_ENABLE \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-Iinterface \
	-Iinterface/driver \
	-Iinterface/driver/device \
	-Iinterface/driver/utils \
	-Iinterface/driver/cpu/br56 \
	-Iinterface/driver/cpu/periph \
	-Iinterface/media \
	-Iinterface/system \
	-Iinterface/system/generic \
	-Iinterface/system/device \
	-Iinterface/utils/jl_ui \
	-Iinterface/btctrler \
	-Iinterface/btctrler/port/br56 \
	-Iinterface/update \
	-Iinterface/agreement \
	-Iinterface/btstack/third_party/common \
	-Iinterface/btstack/third_party \
	-Iinterface/utils \
	-Iinterface/utils/device \
	-Iinterface/utils/fs \
	-Iinterface/utils/generic \
	-Iinterface/utils/math/cpu/br56 \
	-Iinterface/net/mbedtls_3_4_0 \
	-Iinterface/btstack/third_party/rcsp \
	-Iinterface/media/media_new \
	-Iinterface/media/media_new/media \
	-Iinterface/media/media_new/media/cpu/br56 \
	-Iinterface/media/media_new/media/cpu/br56/asm \
	-Iapps/wireless_mic \
	-Iapps/wireless_mic/include \
	-Iapps/wireless_mic/include/wireless_mic \
	-Iapps/wireless_mic/include/jl_ui \
	-Iapps/wireless_mic/board/br56 \
	-Iapps/wireless_mic/ui \
	-Iapps/wireless_mic/demo \
	-Iapps/common \
	-Iapps/common/power_manage \
	-Iapps/common/device \
	-Iapps/common/device/sensor \
	-Iapps/common/device/sensor/ntc \
	-Iapps/common/device/key \
	-Iapps/common/include \
	-Iapps/common/ui \
	-Iapps/common/config/include \
	-Iapps/common/dev_manager \
	-Iapps/common/temp_trim \
	-Iapps/common/third_party_profile \
	-Iapps/common/third_party_profile/common \
	-Iapps/common/third_party_profile/interface \
	-Iapps/common/third_party_profile/jieli \
	-Iapps/common/third_party_profile/jieli/trans_data_demo \
	-Iapps/common/third_party_profile/jieli/online_db \
	-Icpu \
	-Icpu/br56 \
	-Iaudio \
	-Iaudio/cpu/br56 \
	-Iaudio/cpu/br56/audio_dec \
	-Iaudio/common \
	-Iaudio/effect \
	-Iapps/common/third_party_profile/jieli/rcsp \
	-Iapps/common/third_party_profile/swift_pair \
	-Iapps/common/third_party_profile/gfps_protocol \
	-Iapps/common/third_party_profile/custom_protocol_demo \
	-Iapps/common/cJSON \
	-Iinterface/btstack \
	-Iinterface/media/cpu/br56 \
	-Iinterface/utils/flash_translation_layer \
	-Iinterface/ui/jl_ui \
	-Iinterface/ui/jl_ui/ui/cpu/br56 \
	-Iapps/common/le_audio \
	-Iapps/wireless_mic/include/le_audio \
	-Iapps/wireless_mic/include/le_audio/big \
	-Iapps/wireless_mic/include/le_audio/cig \
	-Iinterface/btstack/third_party/le_audio \
	-Iinterface/btstack/third_party/broadcast \
	-Iaudio/le_audio \
	-Iapps/common/device/usb \
	-Iapps/common/device/usb/device \
	-Iapps/common/device/usb/host \
	-Iaudio/interface/include \
	-Iaudio/framework/include \
	-Iaudio/framework/plugs/source \
	-Iaudio/CVP \
	-Iaudio/test_tools \
	-Iaudio/common/icsd/adt \
	-Iaudio/common/icsd/anc \
	-Iaudio/common/icsd/common \
	-Iaudio/common/icsd/dot \
	-Iaudio/common/icsd/common_v2 \
	-Iaudio/common/icsd/anc_v2 \
	-Iaudio/common/icsd/config \
	-Iaudio/common/icsd/rt_anc \
	-Iaudio/common/icsd/tool \
	-Iaudio/common/power \
	-Iaudio/anc \
	-Iaudio/cpu/br56/audio_dai \
	-Iinterface/media/aispeech/enc \
	-Iinterface/media/framework/include \
	-Iinterface/media/cvp \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	apps/common/app_mode_manager/app_mode_manager.c \
	apps/common/bt_common/bt_test_api.c \
	apps/common/clock_manager/clock_manager.c \
	apps/common/config/app_config.c \
	apps/common/config/bt_name_parse.c \
	apps/common/config/bt_profile_config.c \
	apps/common/config/cfg_tool_cdc.c \
	apps/common/config/cfg_tool_statistics_functions/cfg_tool_statistics.c \
	apps/common/config/ci_transport_uart.c \
	apps/common/config/new_cfg_tool.c \
	apps/common/debug/debug.c \
	apps/common/debug/debug_lite.c \
	apps/common/debug/debug_uart_config.c \
	apps/common/device/key/adkey.c \
	apps/common/device/key/iokey.c \
	apps/common/device/key/key_driver.c \
	apps/common/device/key/touch_key.c \
	apps/common/device/storage_device/norflash/norflash_sfc.c \
	apps/common/device/usb/demo/host/udisk_block_test.c \
	apps/common/device/usb/device/cdc.c \
	apps/common/device/usb/device/custom_hid.c \
	apps/common/device/usb/device/descriptor.c \
	apps/common/device/usb/device/hid.c \
	apps/common/device/usb/device/iap.c \
	apps/common/device/usb/device/midi.c \
	apps/common/device/usb/device/msd.c \
	apps/common/device/usb/device/msd_upgrade.c \
	apps/common/device/usb/device/mtp.c \
	apps/common/device/usb/device/printer.c \
	apps/common/device/usb/device/rcsp_hid_inter.c \
	apps/common/device/usb/device/task_pc.c \
	apps/common/device/usb/device/uac1.c \
	apps/common/device/usb/device/uac2.c \
	apps/common/device/usb/device/uac_stream.c \
	apps/common/device/usb/device/usb_device.c \
	apps/common/device/usb/device/usb_pll_trim.c \
	apps/common/device/usb/device/user_setup.c \
	apps/common/device/usb/host/adb.c \
	apps/common/device/usb/host/aoa.c \
	apps/common/device/usb/host/hid.c \
	apps/common/device/usb/host/hub.c \
	apps/common/device/usb/host/uac_host.c \
	apps/common/device/usb/host/usb_bulk_transfer.c \
	apps/common/device/usb/host/usb_ctrl_transfer.c \
	apps/common/device/usb/host/usb_host.c \
	apps/common/device/usb/host/usb_storage.c \
	apps/common/device/usb/usb_config.c \
	apps/common/device/usb/usb_epbuf_manager.c \
	apps/common/device/usb/usb_host_config.c \
	apps/common/device/usb/usb_task.c \
	apps/common/fat_nor/cfg_private.c \
	apps/common/fat_nor/virfat_flash.c \
	apps/common/le_audio/big.c \
	apps/common/le_audio/cig.c \
	apps/common/le_audio/wireless_trans_manager.c \
	apps/common/temp_trim/dtemp_pll_trim.c \
	apps/common/third_party_profile/jieli/online_db/online_db_deal.c \
	apps/common/third_party_profile/jieli/online_db/spp_online_db.c \
	apps/common/third_party_profile/jieli/trans_data_demo/le_trans_data.c \
	apps/common/third_party_profile/jieli/trans_data_demo/spp_trans_data.c \
	apps/common/third_party_profile/multi_protocol_common.c \
	apps/common/third_party_profile/multi_protocol_event.c \
	apps/common/third_party_profile/multi_protocol_main.c \
	apps/common/ui/led7/led7_ui_api.c \
	apps/common/ui/pwm_led/led_ui_api.c \
	apps/common/update/testbox_uart_update.c \
	apps/common/update/testbox_update.c \
	apps/common/update/update.c \
	apps/wireless_mic/app_main.c \
	apps/wireless_mic/audio/jlstream_event_handler.c \
	apps/wireless_mic/audio/scene_switch.c \
	apps/wireless_mic/audio/tone_table.c \
	apps/wireless_mic/battery/battery_level.c \
	apps/wireless_mic/battery/charge.c \
	apps/wireless_mic/battery/charge_device_handle.c \
	apps/wireless_mic/battery/charge_store.c \
	apps/wireless_mic/ble/ble_adv.c \
	apps/wireless_mic/ble/bt_ble.c \
	apps/wireless_mic/board/br56/board_config.c \
	apps/wireless_mic/board/br56/undef_func.c \
	apps/wireless_mic/board/sdk_board_config.c \
	apps/wireless_mic/demo/uart_demo.c \
	apps/wireless_mic/device_config.c \
	apps/wireless_mic/log_config/app_config.c \
	apps/wireless_mic/log_config/lib_btctrler_config.c \
	apps/wireless_mic/log_config/lib_btstack_config.c \
	apps/wireless_mic/log_config/lib_driver_config.c \
	apps/wireless_mic/log_config/lib_media_config.c \
	apps/wireless_mic/log_config/lib_system_config.c \
	apps/wireless_mic/log_config/lib_update_config.c \
	apps/wireless_mic/message/adapter/app_msg.c \
	apps/wireless_mic/message/adapter/audio_player.c \
	apps/wireless_mic/message/adapter/battery.c \
	apps/wireless_mic/message/adapter/driver.c \
	apps/wireless_mic/message/adapter/key.c \
	apps/wireless_mic/mode/bt/bt_app_msg_handler.c \
	apps/wireless_mic/mode/bt/bt_event_func.c \
	apps/wireless_mic/mode/bt/bt_key_func.c \
	apps/wireless_mic/mode/bt/bt_key_msg_table.c \
	apps/wireless_mic/mode/bt/le_audio/big/app_le_broadcast.c \
	apps/wireless_mic/mode/bt/le_audio/big/le_broadcast.c \
	apps/wireless_mic/mode/bt/le_audio/big/le_broadcast_config.c \
	apps/wireless_mic/mode/bt/le_audio/le_audio_buffer.c \
	apps/wireless_mic/mode/bt/le_audio/le_audio_common.c \
	apps/wireless_mic/mode/bt/le_audio/le_audio_custom_data_api.c \
	apps/wireless_mic/mode/bt/poweroff.c \
	apps/wireless_mic/mode/bt/tone.c \
	apps/wireless_mic/mode/bt/wireless_mic.c \
	apps/wireless_mic/mode/common/app_default_msg_handler.c \
	apps/wireless_mic/mode/idle/idle.c \
	apps/wireless_mic/mode/idle/idle_app_msg_handler.c \
	apps/wireless_mic/mode/idle/idle_key_msg_table.c \
	apps/wireless_mic/mode/key_tone.c \
	apps/wireless_mic/mode/mic_effect/mic_effect.c \
	apps/wireless_mic/mode/pc/pc.c \
	apps/wireless_mic/mode/pc/pc_app_msg_handler.c \
	apps/wireless_mic/mode/pc/pc_key_msg_table.c \
	apps/wireless_mic/mode/update/update.c \
	apps/wireless_mic/tools/app_testbox.c \
	apps/wireless_mic/ui/led/led_config.c \
	apps/wireless_mic/ui/led7/ui_bt.c \
	apps/wireless_mic/ui/led7/ui_common.c \
	apps/wireless_mic/ui/led7/ui_idle.c \
	apps/wireless_mic/ui/ui_app_msg_led7.c \
	apps/wireless_mic/ui/ui_app_msg_pwmled.c \
	apps/wireless_mic/user_cfg.c \
	audio/CVP/audio_cvp.c \
	audio/CVP/audio_cvp_3mic.c \
	audio/CVP/audio_cvp_ais_3mic.c \
	audio/CVP/audio_cvp_config.c \
	audio/CVP/audio_cvp_demo.c \
	audio/CVP/audio_cvp_develop.c \
	audio/CVP/audio_cvp_dms.c \
	audio/CVP/audio_cvp_online.c \
	audio/CVP/audio_cvp_ref_task.c \
	audio/CVP/audio_cvp_sync.c \
	audio/common/amplitude_statistic.c \
	audio/common/audio_build_needed.c \
	audio/common/audio_dac_digital_volume.c \
	audio/common/audio_dvol.c \
	audio/common/audio_event_handler.c \
	audio/common/audio_export_demo.c \
	audio/common/audio_general.c \
	audio/common/audio_iis_lrclk_capture.c \
	audio/common/audio_node_config.c \
	audio/common/audio_noise_gate.c \
	audio/common/audio_ns.c \
	audio/common/audio_plc.c \
	audio/common/audio_utils.c \
	audio/common/audio_volume_mixer.c \
	audio/common/bt_audio_energy_detection.c \
	audio/common/debug/audio_debug.c \
	audio/common/demo/audio_alink_demo.c \
	audio/common/demo/audio_eq_demo.c \
	audio/common/demo/codec_demo/audio_enc_file_demo.c \
	audio/common/demo/codec_demo/audio_file_dec_demo.c \
	audio/common/demo/codec_demo/audio_frame_codec_demo.c \
	audio/common/demo/hw_math_v2_demo.c \
	audio/common/frame_length_adaptive.c \
	audio/common/online_debug/aud_data_export.c \
	audio/common/online_debug/aud_mic_dut.c \
	audio/common/online_debug/audio_capture.c \
	audio/common/online_debug/audio_online_debug.c \
	audio/common/pcm_data/sine_pcm.c \
	audio/common/power/mic_power_manager.c \
	audio/common/uartPcmSender.c \
	audio/cpu/br56/audio_accelerator/hw_fft.c \
	audio/cpu/br56/audio_anc.c \
	audio/cpu/br56/audio_config.c \
	audio/cpu/br56/audio_demo/audio_adc_demo.c \
	audio/cpu/br56/audio_demo/audio_dac_demo.c \
	audio/cpu/br56/audio_demo/audio_fft_demo.c \
	audio/cpu/br56/audio_mic_capless.c \
	audio/cpu/br56/audio_setup.c \
	audio/effect/audio_dc_offset_remove.c \
	audio/effect/audio_pitch_speed_api.c \
	audio/effect/audio_voice_changer_api.c \
	audio/effect/bass_treble.c \
	audio/effect/demo/autotune_demo.c \
	audio/effect/demo/bass_treble_demo.c \
	audio/effect/demo/chorus_demo.c \
	audio/effect/demo/crossover_demo.c \
	audio/effect/demo/drc_demo.c \
	audio/effect/demo/dynamic_eq_demo.c \
	audio/effect/demo/echo_demo.c \
	audio/effect/demo/energy_detect_demo.c \
	audio/effect/demo/eq_demo.c \
	audio/effect/demo/frequency_shift_howling_demo.c \
	audio/effect/demo/gain_demo.c \
	audio/effect/demo/harmonic_exciter_demo.c \
	audio/effect/demo/noisegate_demo.c \
	audio/effect/demo/notch_howling_demo.c \
	audio/effect/demo/pitch_speed_demo.c \
	audio/effect/demo/reverb_advance_demo.c \
	audio/effect/demo/reverb_demo.c \
	audio/effect/demo/spectrum_demo.c \
	audio/effect/demo/stereo_widener_demo.c \
	audio/effect/demo/surround_demo.c \
	audio/effect/demo/virtual_bass_demo.c \
	audio/effect/demo/voice_changer_demo.c \
	audio/effect/effects_adj.c \
	audio/effect/effects_default_param.c \
	audio/effect/effects_dev.c \
	audio/effect/eq_config.c \
	audio/effect/esco_ul_voice_changer.c \
	audio/effect/node_param_update.c \
	audio/effect/scene_update.c \
	audio/effect/spk_eq.c \
	audio/framework/nodes/agc_node.c \
	audio/framework/nodes/ai_tx_node.c \
	audio/framework/nodes/channle_swap_node.c \
	audio/framework/nodes/cvp_3mic_node.c \
	audio/framework/nodes/cvp_develop_node.c \
	audio/framework/nodes/cvp_dms_node.c \
	audio/framework/nodes/cvp_sms_node.c \
	audio/framework/nodes/data_export_node.c \
	audio/framework/nodes/dns_node.c \
	audio/framework/nodes/effect_dev0_node.c \
	audio/framework/nodes/effect_dev1_node.c \
	audio/framework/nodes/effect_dev2_node.c \
	audio/framework/nodes/effect_dev3_node.c \
	audio/framework/nodes/effect_dev4_node.c \
	audio/framework/nodes/esco_tx_node.c \
	audio/framework/nodes/iis_node.c \
	audio/framework/nodes/le_audio_source.c \
	audio/framework/nodes/lhdc_x_node.c \
	audio/framework/nodes/local_tws_source_node.c \
	audio/framework/nodes/multi_ch_iis_node.c \
	audio/framework/nodes/ns_node.c \
	audio/framework/nodes/pc_mic_node.c \
	audio/framework/nodes/plc_node.c \
	audio/framework/nodes/sink_dev0_node.c \
	audio/framework/nodes/sink_dev1_node.c \
	audio/framework/nodes/sink_dev2_node.c \
	audio/framework/nodes/sink_dev3_node.c \
	audio/framework/nodes/sink_dev4_node.c \
	audio/framework/nodes/surround_demo_node.c \
	audio/framework/nodes/uart_node.c \
	audio/framework/nodes/volume_node.c \
	audio/framework/plugs/source/a2dp_file.c \
	audio/framework/plugs/source/a2dp_streamctrl.c \
	audio/framework/plugs/source/adc_file.c \
	audio/framework/plugs/source/esco_file.c \
	audio/framework/plugs/source/iis_file.c \
	audio/framework/plugs/source/le_audio_file.c \
	audio/framework/plugs/source/linein_file.c \
	audio/framework/plugs/source/local_tws_file.c \
	audio/framework/plugs/source/multi_ch_iis_file.c \
	audio/framework/plugs/source/pc_spk_file.c \
	audio/framework/plugs/source/pdm_mic_file.c \
	audio/framework/plugs/source/source_dev0_file.c \
	audio/framework/plugs/source/source_dev1_file.c \
	audio/framework/plugs/source/source_dev2_file.c \
	audio/framework/plugs/source/source_dev3_file.c \
	audio/framework/plugs/source/source_dev4_file.c \
	audio/interface/player/a2dp_player.c \
	audio/interface/player/adda_loop_player.c \
	audio/interface/player/dev_flow_player.c \
	audio/interface/player/esco_player.c \
	audio/interface/player/file_player.c \
	audio/interface/player/iis_player.c \
	audio/interface/player/key_tone_player.c \
	audio/interface/player/le_audio_player.c \
	audio/interface/player/linein_player.c \
	audio/interface/player/local_tws_player.c \
	audio/interface/player/mic_player.c \
	audio/interface/player/pc_spk_player.c \
	audio/interface/player/reference_time.c \
	audio/interface/player/ring_player.c \
	audio/interface/player/tone_player.c \
	audio/interface/recoder/ai_voice_recoder.c \
	audio/interface/recoder/dev_flow_recoder.c \
	audio/interface/recoder/esco_recoder.c \
	audio/interface/recoder/file_recorder.c \
	audio/interface/recoder/le_audio_mix_mic_recorder.c \
	audio/interface/recoder/le_audio_recorder.c \
	audio/interface/recoder/pc_mic_recoder.c \
	audio/jl_kws/jl_kws_algo.c \
	audio/jl_kws/jl_kws_audio.c \
	audio/jl_kws/jl_kws_event.c \
	audio/jl_kws/jl_kws_main.c \
	audio/le_audio/le_audio_stream.c \
	audio/midi/audio_dec_midi_ctrl.c \
	audio/midi/audio_dec_midi_file.c \
	audio/test_tools/audio_dut_control.c \
	audio/test_tools/audio_dut_control_old.c \
	audio/test_tools/audio_enc_mpt_cvp_ctr.c \
	audio/test_tools/audio_enc_mpt_self.c \
	audio/test_tools/cvp_tool.c \
	audio/test_tools/mic_dut_process.c \
	cpu/br56/charge/charge.c \
	cpu/br56/charge/charge_config.c \
	cpu/br56/charge/charge_ocp.c \
	cpu/br56/charge/chargestore.c \
	cpu/br56/charge/chargestore_config.c \
	cpu/br56/lua_port_api.c \
	cpu/br56/overlay_code.c \
	cpu/br56/periph/led/pwm_led.c \
	cpu/br56/periph/mcpwm.c \
	cpu/br56/periph/rdec.c \
	cpu/br56/periph/touch/lp_touch_key.c \
	cpu/br56/periph/touch/lp_touch_key_identify_algo.c \
	cpu/br56/periph/touch/lp_touch_key_range_algo.c \
	cpu/br56/periph/touch/lp_touch_key_tool.c \
	cpu/br56/periph/touch/lpctmu_hw.c \
	cpu/br56/power/key_wakeup.c \
	cpu/br56/power/power_app.c \
	cpu/br56/power/power_config.c \
	cpu/br56/power/power_gate.c \
	cpu/br56/power/power_port.c \
	cpu/br56/rf_api.c \
	cpu/br56/setup.c \
	cpu/components/iic_api.c \
	cpu/components/iic_soft.c \
	cpu/components/ir_decoder.c \
	cpu/components/ir_encoder.c \
	cpu/components/led_api.c \
	cpu/components/rdec_soft.c \
	cpu/components/two_io_led.c \
	cpu/components/ui_driver/led7/led7_driver.c \
	cpu/components/ui_driver/ui_common.c \
	cpu/config/gpio_file_parse.c \
	cpu/config/lib_power_config.c \
	cpu/periph_demo/gptimer_demo.c \
	cpu/periph_demo/iic_master_demo.c \
	cpu/periph_demo/iic_slave_demo.c \
	cpu/periph_demo/ir_encoder_decoder_demo.c \
	cpu/periph_demo/led_api_test.c \
	cpu/periph_demo/ledc_test.c \
	cpu/periph_demo/pwm_led_test.c \
	cpu/periph_demo/rdec_soft_demo.c \
	cpu/periph_demo/sd_test.c \
	cpu/periph_demo/spi_test.c \
	cpu/periph_demo/two_io_led_test.c \
	cpu/periph_demo/uart_test.c \


# 需要编译的 .S 文件
S_SRC_FILES := \
	apps/wireless_mic/sdk_version.z.S \


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-pi32v2-always-use-itblock=false \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-merge-max-offset=4096 \
	--plugin-opt=-pi32v2-enable-simd=true \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-inline-max-allocated-size=32 \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-enable-rep-memop \
	--plugin-opt=-warn-stack-size=1024 \
	--sort-common \
	--dont-complain-call-overflow \
	--plugin-opt=-enable-movable-region=true \
	--plugin-opt=-movable-region-section-prefix=.movable.slot. \
	--plugin-opt=-movable-region-stub-section-prefix=.movable.stub. \
	--plugin-opt=-movable-region-prefix=.movable.region. \
	--plugin-opt=-movable-region-stub-prefix=__movable_stub_ \
	--plugin-opt=-movable-region-stub-swi-number=-2 \
	--plugin-opt=-movable-region-map-file-list=apps/wireless_mic/movable/funcname.txt \
	--plugin-opt=-movable-region-section-map-file-list=apps/wireless_mic/movable/section.txt \
	--plugin-opt=-movable-region-exclude-func-file-list=apps/wireless_mic/movable/exclude.txt \
	--plugin-opt=-used-symbol-file=apps/wireless_mic/sdk_used_list.used \
	--plugin-opt=-floating-point-may-trap=true \
	--gc-sections \
	--plugin-opt=-pi32v2-csync-before-rti=true \
	--start-group \
	cpu/br56/liba/cpu.a \
	cpu/br56/liba/system.a \
	cpu/br56/liba/btctrler.a \
	cpu/br56/liba/aec.a \
	cpu/br56/liba/media.a \
	cpu/br56/liba/wireless_mic_lib.a \
	cpu/br56/liba/libAptFilt_pi32v2_OnChip.a \
	cpu/br56/liba/libEchoSuppress_pi32v2_OnChip.a \
	cpu/br56/liba/libNoiseSuppress_pi32v2_OnChip.a \
	cpu/br56/liba/libSplittingFilter_pi32v2_OnChip.a \
	cpu/br56/liba/libDualMicSystem_pi32v2_OnChip.a \
	cpu/br56/liba/libDualMicSystem_flexible_pi32v2_OnChip.a \
	cpu/br56/liba/libSMS_TDE_pi32v2_OnChip.a \
	cpu/br56/liba/agreement.a \
	cpu/br56/liba/lib_advaudio_plc.a \
	cpu/br56/liba/SpectrumShow.a \
	cpu/br56/liba/DynamicEQ.a \
	cpu/br56/liba/DynamicEQPro.a \
	cpu/br56/liba/lib_voiceChanger.a \
	cpu/br56/liba/lib_SurroundEffect.a \
	cpu/br56/liba/howling.a \
	cpu/br56/liba/noisegate.a \
	cpu/br56/liba/lib_noisegate_pro.a \
	cpu/br56/liba/lib_FrequencyShift.a \
	cpu/br56/liba/crossover_coff.a \
	cpu/br56/liba/lib_reverb_cal.a \
	cpu/br56/liba/lib_plate_reverb_lite.a \
	cpu/br56/liba/lib_pitch_speed.a \
	cpu/br56/liba/lib_esco_audio_plc.a \
	cpu/br56/liba/lib_adpcm_code.a \
	cpu/br56/liba/lib_adpcm_ima_code.a \
	cpu/br56/liba/lib_StereoWidener.a \
	cpu/br56/liba/lib_HarmonicExciter.a \
	cpu/br56/liba/lib_resample_fast_cal.a \
	cpu/br56/liba/lib_Chorus.a \
	cpu/br56/liba/VirtualBass.a \
	cpu/br56/liba/lib_virtual_bass_classic.a \
	cpu/br56/liba/lib_plateReverb_adv.a \
	cpu/br56/liba/drc.a \
	cpu/br56/liba/lib_drc_detect.a \
	cpu/br56/liba/lib_drc_advance.a \
	cpu/br56/liba/lib_PcmDelay.a \
	cpu/br56/liba/lib_speex_codec.a \
	cpu/br56/liba/encryption.a \
	cpu/br56/liba/lib_PingPongDelay.a \
	cpu/br56/liba/lib_ThreeD_effect.a \
	cpu/br56/liba/lib_PingPongDelay.a  \
	cpu/br56/liba/lib_ThreeD_effect.a  \
	cpu/br56/liba/libjlsp.a \
	cpu/br56/liba/lib_StereoMtapsEcho.a \
	cpu/br56/liba/lib_HowlingGate.a \
	cpu/br56/liba/crossover_coff.a  \
	cpu/br56/liba/lib_limiter.a \
	cpu/br56/liba/lib_jl_codec_common.a \
	cpu/br56/liba/lib_jla_codec.a \
	cpu/br56/liba/lib_jla_v2_codec.a \
	cpu/br56/liba/lib_jla_ll_codec.a \
	cpu/br56/liba/lib_phaser.a \
	cpu/br56/liba/lib_flanger.a \
	cpu/br56/liba/lib_chorus_adv.a \
	cpu/br56/liba/lib_pingpong_echo.a \
	cpu/br56/liba/lib_llns_dns.a \
	cpu/br56/liba/libllns.a \
	cpu/br56/liba/libFFT_pi32v2_OnChip.a \
	cpu/br56/liba/lib_wtg_dec.a \
	cpu/br56/liba/lib_bt_aac_rom_dec.a \
	cpu/br56/liba/lib_mp3_rom_dec.a \
	cpu/br56/liba/crypto_toolbox_Osize.a \
	cpu/br56/liba/update.a \
	cpu/br56/liba/agreement.a  \
	cpu/br56/liba/cbuf.a \
	cpu/br56/liba/chargestore.a \
	cpu/br56/liba/lbuf.a \
	cpu/br56/liba/printf.a \
	cpu/br56/liba/device.a \
	cpu/br56/liba/fs.a \
	cpu/br56/liba/ascii.a \
	cpu/br56/liba/cfg_tool.a \
	cpu/br56/liba/vm.a \
	cpu/br56/liba/ftl.a \
	cpu/br56/liba/debug_record.a \
	cpu/br56/liba/chargestore.a  \
	--end-group \
	-Tcpu/br56/sdk.ld \
	-M=cpu/br56/tools/sdk.map \
	--plugin-opt=mcpu=r3 \
	--plugin-opt=-mattr=+fprev1 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/wireless_mic/sdk_used_list.c -o apps/wireless_mic/sdk_used_list.used
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/wireless_mic/movable/section.c -o apps/wireless_mic/movable/section.txt
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br56/sdk_ld.c -o cpu/br56/sdk.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br56/tools/download.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br56/tools/isd_config_rule.c -o cpu/br56/tools/isd_config.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

-include $(DEP_FILES)
