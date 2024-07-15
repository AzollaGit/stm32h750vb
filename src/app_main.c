/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/sys/printk.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>


#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_rtc.h"
#include "hal_pwm.h"
#include "qspi_flash.h"
#include "fs_nvs.h"
#include "sd_disk.h"
#include "bsp_sd.h"
#include "cpu_temp.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_main, LOG_LEVEL_DBG);

void uart_recv_callback(uart_port_t uart_port, uint8_t *data, uint16_t len)
{
	LOG_INF("port[%d] = %s | %d", uart_port, data, len);
}
 
extern void led0_toggle(void);
 
extern int hal_adc_setup(void);


int main(void)
{
	LOG_DBG("app main start...");
	// k_msleep(1000);

 	hal_gpio_init();

	hal_uart_init(uart_recv_callback);

	spi_flash_init();

	fs_nvs_init();
  
	hal_pwm_setup();

	rtc_write_time(1767225595UL);

#if DT_HAS_COMPAT_STATUS_OKAY(st_stm32_sdmmc)
// #if DT_NODE_HAS_STATUS(DT_PATH(soc, sdmmc_52007000), okay)
	sd_disk_init();
#else
	bap_sd_init();
#endif

	cpu_temp_init();
 
	while (1) {

        led0_toggle();

		static uint8_t pwm_duty = 0;
		if (++pwm_duty > 100) pwm_duty = 1;
		hal_pwm_set_duty(pwm_duty);

		(void)cpu_temp_read();
  
		#if 0
		struct rtc_time datetime_get;
		time_t timer_get = rtc_read_time(&datetime_get);
		LOG_DBG("datetime_get.tm_sec = %d, %lld", datetime_get.tm_sec, timer_get);
		#endif
        k_msleep(1000);
		//break;    
    }

	hal_adc_setup();
 
	return 0; 
}

/** 开发板资料：
 * http://124.222.62.86/yd-data/YD-STM32H7xxvx/
*/