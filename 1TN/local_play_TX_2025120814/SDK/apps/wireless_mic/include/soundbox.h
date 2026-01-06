#ifndef EARPHONE_H
#define EARPHONE_H

#include "system/includes.h"

struct sys_event;

extern struct bt_mode_var g_bt_hdl;

extern u8 bt_app_exit_check(void);
extern const struct key_remap_table *get_key_remap_table();
struct app_mode *app_enter_bt_mode(int arg);
bool bt_check_already_initializes(void);
void btstack_init_in_other_mode(void);
void btstack_exit_in_other_mode(void);
void btstack_init_for_app(void);
void btstack_exit_for_app(void);

void wireless_mic_audio_status_reset(void);
bool get_llns_dns_bypass_status(void);
bool get_llns_bypass_status(void);
bool get_plate_reverb_bypass_status(void);
#endif
