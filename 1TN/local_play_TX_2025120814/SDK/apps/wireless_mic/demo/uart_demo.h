#ifndef __UART_DEMO_H__
#define __UART_DEMO_H__

void uart_demo_init(void);
void uart_demo_close(void);
void uart_duplex_send_data(u8 *buf, u32 len);
void uart_duplex_get_data(void);

#endif


