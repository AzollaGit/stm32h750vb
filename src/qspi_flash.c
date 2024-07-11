/* com
*/
#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/flash/stm32_flash_api_extensions.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>

#include "qspi_flash.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(qspi_flash, LOG_LEVEL_DBG);


#define DT_DRV_COMPAT 	st_stm32_qspi_nor
 
/* Get the base address of the flash from the DTS node */
#define STORAGE_LABEL		    	storage	
#define STORAGE_PARTITION_OFFSET	DT_REG_ADDR(DT_NODE_BY_FIXED_PARTITION_LABEL(STORAGE_LABEL)) 
#define STORAGE_PARTITION_SIZE 		DT_REG_SIZE(DT_NODE_BY_FIXED_PARTITION_LABEL(STORAGE_LABEL))
#define OFFSET_ADDR_CHECK(offset, size) \
	 	if (offset + size > STORAGE_PARTITION_SIZE)  { \
			LOG_WRN("offset + size > %d", STORAGE_PARTITION_SIZE); \
			return -EMSGSIZE; }

#define SPI_FLASH_SECTOR_SIZE        	4096
static const struct device *flash_dev = DEVICE_DT_GET_ONE(DT_DRV_COMPAT);
// const struct device *flash_dev = FIXED_PARTITION_DEVICE(storage_partition);
// const struct device *flash_dev = FIXED_PARTITION_DEVICE(nvs_partition);
// const struct device *flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));

int spi_flash_erase(off_t offset, size_t size)
{	
	OFFSET_ADDR_CHECK(offset, size);
	int ret = flash_erase(flash_dev, STORAGE_PARTITION_OFFSET + offset, size);
	if (ret != 0) {
		LOG_WRN("Flash erase failed! %d", ret);
	}
	return ret;
}

int spi_flash_write(off_t offset, const void *data, size_t len)
{	
	OFFSET_ADDR_CHECK(offset, len);
	int ret = flash_write(flash_dev, STORAGE_PARTITION_OFFSET + offset, data, len);
	if (ret != 0) {
		LOG_WRN("Flash write failed! %d", ret);
	}
	return ret;
}

int spi_flash_read(off_t offset, void *data, size_t len)
{	
	OFFSET_ADDR_CHECK(offset, len);
	int ret = flash_read(flash_dev, STORAGE_PARTITION_OFFSET + offset, data, len);
	if (ret != 0) {
		LOG_WRN("Flash read failed! %d", ret);
	}
	return ret;
}


void spi_flash_test(const struct device *flash_dev)
{
	const uint8_t expected[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	int ret;
 
	/* Full flash erase if STORAGE_PARTITION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	ret = flash_erase(flash_dev, STORAGE_PARTITION_OFFSET, SPI_FLASH_SECTOR_SIZE);
	if (ret != 0) {
		LOG_WRN("Flash erase failed! %d", ret);
	} else {
		LOG_DBG("Flash erase succeeded!");
	}
	
	LOG_DBG("Attempting to write %zu bytes", len);
	ret = flash_write(flash_dev, STORAGE_PARTITION_OFFSET, expected, len);
	if (ret != 0) {
		LOG_WRN("Flash write failed! %d", ret);
		return;
	}
 
	memset(buf, 0, len);
	ret = flash_read(flash_dev, STORAGE_PARTITION_OFFSET, buf, len);
	if (ret != 0) {
		LOG_WRN("Flash read failed! %d", ret);
		return;
	}
	LOG_HEXDUMP_DBG(buf, len, "flash_read");
}
 
int spi_flash_init(void)
{
	if (!device_is_ready(flash_dev)) {
		LOG_WRN("%s: device not ready.", flash_dev->name);
		return -ENODEV;
	}
	
	LOG_DBG("%s SPI flash testing", flash_dev->name);
	 
	LOG_DBG("storage_partition@ 0x%08x, size: 0x%x", 
				STORAGE_PARTITION_OFFSET, 
				STORAGE_PARTITION_SIZE);
 
	spi_flash_test(flash_dev);
	return 0;
} 