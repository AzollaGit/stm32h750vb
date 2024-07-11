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
#include "hal_uart.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hal_uart, LOG_LEVEL_DBG);
 
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(usart1));

typedef struct {
#define UART1_MSG_SIZE	 64
    uint8_t  buff[UART1_MSG_SIZE];
    uint16_t size;
} uart_rxd_t;

static uart_rxd_t rxd = { 0 };

static struct k_work_delayable dwork;

static uart_recv_callback_t uart_recv_callback = NULL; 
 
 
void uart1_write(const uint8_t *buf, size_t len)
{
	for (int i = 0; i < len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

void uart1_string(const char *string)
{
	int len = strlen(string);
	uart1_write(string, len);
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
  
	/* read until FIFO empty */
	uint8_t data;
	while (uart_fifo_read(uart_dev, &data, 1) == 1) {
		if (data == '#') {
			printk("\r\n	---- cpu stop ----\r\n\r\n");
			while (1);  // 用来卡死烧写用的，这样就不用按BOOT + RST按键烧写了！
		} 
	 
		if (!rxd.size && !k_work_delayable_is_pending(&dwork)) {
			k_work_reschedule(&dwork, K_MSEC(10));  // 延时10ms读取接收结果！
		}

		if (rxd.size < UART1_MSG_SIZE) {
			rxd.buff[rxd.size++] = data;
		}
		
	}
}
 
// UART延时接收工作任务
void uart_read_work(struct k_work *work)
{
	// LOG_DBG("uart_read_work...");
	uart_recv_callback(UART1_PORT, rxd.buff, rxd.size);
	memset(rxd.buff, 0, rxd.size);
	rxd.size = 0;
}
 
int hal_uart_init(uart_recv_callback_t cb)
{
	if (!device_is_ready(uart_dev)) {
		LOG_WRN("UART device not found!");
		return -ENODEV;
	}

	/* configure interrupt and callback to receive data */
	int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			LOG_WRN("Interrupt-driven UART API support not enabled");
		} else if (ret == -ENOSYS) {
			LOG_WRN("UART device does not support interrupt-driven API");
		} else {
			LOG_WRN("Error setting UART callback: %d\n", ret);
		}
		return ret;
	}
	uart_irq_rx_enable(uart_dev);

    uart_recv_callback = cb;
 
	k_work_init_delayable(&dwork, uart_read_work);
 
	return 0;
}

 