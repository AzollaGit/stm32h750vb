/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
 
 
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include "hal_uart.h"

LOG_MODULE_REGISTER(uart, LOG_LEVEL_DBG);
 
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(usart1));

 
/* queue to store up to 3 messages (aligned to 4-byte boundary) */
#define UART1_MSG_SIZE    64
K_MSGQ_DEFINE(uart_msgq, UART1_MSG_SIZE, 3, 4);

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);
	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	if (!uart_irq_update(uart_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}

	uint8_t c;
    /* receive buffer used in UART ISR callback */
    static uart_data_t rx = { 0 }; 
    if (rx.data == NULL) rx.data = k_malloc(UART1_MSG_SIZE);

    /* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if (c == '\n' && rx.len > 0) {
			/* terminate string */
			rx.data[rx.len] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx.len = 0;
		} else if (rx.len < (UART1_MSG_SIZE - 1)) {
			rx.data[rx.len++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}



#define UART_STACK_SIZE     1024
K_THREAD_STACK_DEFINE(uart_stack, UART_STACK_SIZE);
struct k_thread uart_thread;

static uart_recv_callback_t uart_recv_callback = NULL; 
 
 void uart_read_thread(void *p1, void *p2, void *p3)
{
    uart_data_t rx = { 0 }; 
    if (rx.data == NULL) rx.data = k_malloc(UART1_MSG_SIZE);
    LOG_INF("<%s>, uart1...", __func__);
    /* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &rx, K_FOREVER) == 0) {
        //LOG_INF("uart1_rx = %s | %d", rx.data, rx.len);
        uart_recv_callback(UART1_PORT, rx.data, rx.len);
	}
    k_free(rx.data);
}
 
int hal_uart_init(uart_recv_callback_t cb)
{
	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return -EPERM;
	}

	/* configure interrupt and callback to receive data */
	int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return ret;
	}
	uart_irq_rx_enable(uart_dev);
 
    uart_recv_callback = cb;
 
    k_thread_create(&uart_thread, uart_stack, UART_STACK_SIZE, uart_read_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
 
	return 0;
}

 