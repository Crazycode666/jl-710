#include "asm/power_interface.h"
#include "iokey.h"
#include "irkey.h"
#include "adkey.h"
#include "app_config.h"

void gpio_config_soft_poweroff(void)
{
    PORT_TABLE(g);

#if TCFG_IOKEY_ENABLE
    PORT_PROTECT(get_iokey_power_io());
#endif

#if TCFG_ADKEY_ENABLE
    PORT_PROTECT(get_adkey_io());
#endif


#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) && !(TCFG_CHARGE_ENABLE && (TCFG_CHARGESTORE_PORT == IO_PORTP_00)))
    PORT_PROTECT(TCFG_CHARGESTORE_PORT);
#endif

    __port_init((u32)gpio_config);
}
