#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".uart_test.data.bss")
#pragma data_seg(".uart_test.data")
#pragma const_seg(".uart_test.text.const")
#pragma code_seg(".uart_test.text")
#endif
#include "system/includes.h"
#include "uart.h"
#include "debug.h"
#include "app_config.h"

#if TCFG_USER_UART_DEMO_EN

#define DEMO_UART_ID  1

static void *uart_dma_cbuffer;
static void *uart_tx_ptr;
static void *uart_rx_ptr;

static int uart_bus;
static volatile u8 flag_uart_write_busy = 0;
void uart_duplex_get_data(void)
{
    if (uart_bus >= 0 && uart_rx_ptr) {
        uart_recv_bytes(DEMO_UART_ID, uart_rx_ptr, UART_DEMO_RX_BUF_SIZE);
        /* put_buf(uart_rx_ptr, UART_DEMO_RX_BUF_SIZE); */
    }
}

static void uart_isr_hook(int uart_num, enum uart_event event)
{
    if (event & UART_EVENT_RX_DATA) {
        printf("uart[%d] tx done\n", uart_num);
        uart_duplex_get_data();
    }

    if (event & UART_EVENT_RX_TIMEOUT) {
        printf("uart[%d] rx timerout data\n", uart_num);
        uart_duplex_get_data();
    }

    if (event & UART_EVENT_RX_FIFO_OVF) {
        printf("uart[%d] rx fifo ovf\n", uart_num);
        uart_duplex_get_data();
    }
}

void uart_duplex_send_data(u8 *buf, u32 len)
{
    if (!uart_tx_ptr) {
        printf("uart_tx_ptr is NULL\n");
        return;
    }

    if (len > UART_DEMO_RX_BUF_SIZE) {
        printf("data len over limit : %d\n", UART_DEMO_RX_BUF_SIZE);
        return;
    }

    memcpy(uart_tx_ptr, buf, len);

    if (uart_bus >= 0) {
        flag_uart_write_busy = 1;
        uart_send_blocking(DEMO_UART_ID, uart_tx_ptr, len, 0);
        flag_uart_write_busy = 0;
    }
}

void uart_demo_close(void)
{
    printf("%s\n", __func__);
    if (uart_bus >= 0) {
        uart_deinit(0);
        gpio_set_mode(IO_PORT_SPILT(UART_DEMO_TX_PIN), PORT_HIGHZ);
    }

    if (uart_dma_cbuffer) {
        dma_free(uart_dma_cbuffer);
        uart_dma_cbuffer = 0;
    }

    if (uart_tx_ptr) {
        dma_free(uart_tx_ptr);
        uart_tx_ptr = 0;
    }

    if (uart_rx_ptr) {
        dma_free(uart_rx_ptr);
        uart_rx_ptr = 0;
    }
}

void uart_demo_init(void)
{
    const struct uart_config config = {
        .baud_rate = USER_UART_DEMO_BAUD,
        .tx_pin = UART_DEMO_TX_PIN,
        .rx_pin = UART_DEMO_RX_PIN,
        .parity = UART_PARITY_DISABLE,
        .tx_wait_mutex = 0,//1:不支持中断调用,互斥,0:支持中断,不互斥
    };

    uart_dma_cbuffer = dma_malloc(UART_DEMO_RX_BUF_SIZE);
    uart_tx_ptr = dma_malloc(UART_DEMO_RX_BUF_SIZE);
    uart_rx_ptr = dma_malloc(UART_DEMO_RX_BUF_SIZE);
    memset(uart_rx_ptr, 0, UART_DEMO_RX_BUF_SIZE);

    const struct uart_dma_config dma = {
        .rx_timeout_thresh = 3 * 10000000 / config.baud_rate, //单位:us,公式：3*10000000/baud(ot:3个byte时间)
        .event_mask = UART_EVENT_RX_DATA | UART_EVENT_RX_TIMEOUT | UART_EVENT_RX_FIFO_OVF,
        .irq_priority = 2,
        .irq_callback = uart_isr_hook,
        .rx_cbuffer = uart_dma_cbuffer,
        .rx_cbuffer_size = UART_DEMO_RX_BUF_SIZE,
        .frame_size = UART_DEMO_RX_BUF_SIZE,
    };
    int r;
    uart_bus = uart_init(DEMO_UART_ID, &config);
    if (uart_bus < 0) {
        printf("uart(%d) init error\n", uart_bus);
    } else {
        printf("uart(%d) init ok\n", uart_bus);
    }
    r = uart_dma_init(DEMO_UART_ID, &dma);
    if (r < 0) {
        printf("uart(%d) dma init error\n", uart_bus);
    } else {
        printf("uart(%d) dma init ok\n", uart_bus);
    }
}

#endif

