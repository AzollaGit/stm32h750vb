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
 
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_main, LOG_LEVEL_DBG);

void uart_recv_callback(uart_port_t uart_port, uint8_t *data, uint16_t len)
{
	LOG_INF("port[%d] = %s | %d", uart_port, data, len);
}
 
extern void led0_toggle(void);

#if 1
#include <zephyr/drivers/flash.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>
 
#define SPI_FLASH_TEST_REGION_OFFSET 	0x0000
#define SPI_FLASH_SECTOR_SIZE        	4096
 

void single_sector_test(const struct device *flash_dev)
{
	const uint8_t expected[] = { 0x55, 0xAA, 0x0F, 0xF0 };
	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	int rc;
#if CONFIG_FLASH_JESD216_API
	uint8_t id[4];
	rc = flash_read_jedec_id(flash_dev, id);
	LOG_DBG("flash_read_jedec_id = %d", rc);
	LOG_HEXDUMP_DBG(id, 4, "id");
#endif
	LOG_DBG("Perform test on single sector");
	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */
	LOG_DBG("Test 1: Flash erase");
 
	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		LOG_WRN("Flash erase failed! %d", rc);
	} else {
		LOG_DBG("Flash erase succeeded!");
	}
	
	LOG_DBG("Attempting to write %zu bytes", len);
	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected, len);
	if (rc != 0) {
		LOG_WRN("Flash write failed! %d", rc);
		return;
	}
 
	memset(buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
	if (rc != 0) {
		LOG_WRN("Flash read failed! %d", rc);
		return;
	}
	LOG_HEXDUMP_DBG(buf, len, "flash_read");
	// LOG_DBG("flash_read: [%02x, %02x, %02x, %02x]", buf[0], buf[1], buf[2], buf[3]);
 
	size_t flash_block_size;
	size_t total_pages;
	total_pages = flash_get_page_count(flash_dev);
	flash_block_size = flash_get_write_block_size(flash_dev);
	printk("Nand flash driver block size %d, %d\n", flash_block_size, total_pages);
}
#endif

#include "hal_rtc.h"
#include "hal_pwm.h"
 
extern int hal_adc_setup(void);


#if DT_HAS_COMPAT_STATUS_OKAY(jedec_spi_nor)
#define SPI_FLASH_COMPAT jedec_spi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_qspi_nor)
#define SPI_FLASH_COMPAT st_stm32_qspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_ospi_nor)
#define SPI_FLASH_COMPAT st_stm32_ospi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_xspi_nor)
#define SPI_FLASH_COMPAT st_stm32_xspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_qspi_nor)
#define SPI_FLASH_COMPAT nordic_qspi_nor
#else
#define SPI_FLASH_COMPAT invalid
#endif

extern int flash_stm32_setup(void);

int main(void)
{
	LOG_DBG("app main start...");
	// k_msleep(1000);

 	hal_gpio_init();

	hal_uart_init(uart_recv_callback);

#if 1
	// const struct device *flash_dev = DEVICE_DT_GET_ONE(SPI_FLASH_COMPAT);
	const struct device *flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
	if (!device_is_ready(flash_dev)) {
		LOG_WRN("%s: device not ready.", flash_dev->name);
		return 0;
	}
	LOG_DBG("%s SPI flash testing", flash_dev->name);
	LOG_DBG("==========================");
	single_sector_test(flash_dev);
#else
	flash_stm32_setup();
#endif

	uint32_t period = 20000000ul;
	hal_pwm_setup();
	hal_pwm_set_duty(period, period/2);
 
	rtc_write_time(1767225595UL);

	while (1) {

        led0_toggle();
 
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