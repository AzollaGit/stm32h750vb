/* com
*/
#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/flash/stm32_flash_api_extensions.h>
#include <zephyr/devicetree.h>
#include <zephyr/storage/flash_map.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(soc_flash, LOG_LEVEL_DBG);

#if defined(CONFIG_FLASH_STM32_WRITE_PROTECT)

#define TEST_AREA	            storage_partition
#define TEST_AREA_OFFSET        FIXED_PARTITION_OFFSET(TEST_AREA)
#define TEST_AREA_SIZE	        FIXED_PARTITION_SIZE(TEST_AREA)
#define TEST_AREA_MAX	        (TEST_AREA_OFFSET + TEST_AREA_SIZE)
#define TEST_AREA_DEVICE        FIXED_PARTITION_DEVICE(TEST_AREA)
#define TEST_AREA_DEVICE_REG    DT_REG_ADDR(DT_MTD_FROM_FIXED_PARTITION(DT_NODELABEL(TEST_AREA)))

#define EXPECTED_SIZE           16

static const struct device *const flash_dev = TEST_AREA_DEVICE;

static const struct flash_parameters *flash_params;
static uint32_t sector_mask;
static uint8_t __aligned(4) expected[EXPECTED_SIZE];

static int sector_mask_from_offset(const struct device *dev, off_t offset,
				   size_t size, uint32_t *mask)
{
	struct flash_pages_info start_page, end_page;

	if (flash_get_page_info_by_offs(dev, offset, &start_page) ||
	    flash_get_page_info_by_offs(dev, offset + size - 1, &end_page)) {
		return -EINVAL;
	}

	*mask = ((1UL << (end_page.index + 1)) - 1) &
		~((1UL << start_page.index) - 1);

	return 0;
}
 
void test_stm32_write_protection(void)
{
	struct flash_stm32_ex_op_sector_wp_in wp_request;
	uint8_t buf[EXPECTED_SIZE];
	int rc;

    sector_mask = 1;

	LOG_DBG("Enabling write protection...");
	wp_request.disable_mask = 0;
	wp_request.enable_mask = sector_mask;

	rc = flash_ex_op(flash_dev, FLASH_STM32_EX_OP_SECTOR_WP, (uintptr_t)&wp_request, NULL);
	if (rc) LOG_WRN("Cannot enable write protection, error %d\n", rc);
    // #define ENOSYS 88	/* Function not implemented */
	else LOG_DBG("Done\n");

    memset(expected, 0xA6, EXPECTED_SIZE);
	rc = flash_write(flash_dev, TEST_AREA_OFFSET, expected, EXPECTED_SIZE);
	LOG_DBG("flash_write, error %d\n", rc);

    while (1) {
        rc = flash_read(flash_dev, TEST_AREA_OFFSET, buf, EXPECTED_SIZE);
        k_msleep(1000);
    }
	
 
    LOG_HEXDUMP_DBG(buf, EXPECTED_SIZE, "text_read1");

	LOG_DBG("Disabling write protection...");
	wp_request.disable_mask = sector_mask;
	wp_request.enable_mask = 0;

	rc = flash_ex_op(flash_dev, FLASH_STM32_EX_OP_SECTOR_WP, (uintptr_t)&wp_request, NULL);
	if (rc) LOG_WRN("Cannot disable write protection, error %d\n", rc);
	else LOG_DBG("Done\n");

}

int flash_stm32_setup(void)
{
	int rc;

    if (!device_is_ready(flash_dev)) {
		LOG_WRN("%s: device not ready.", flash_dev->name);
		return -1;
	}

    LOG_DBG("%s SPI flash testing", flash_dev->name);
 
    flash_params = flash_get_parameters(flash_dev);

	rc = sector_mask_from_offset(flash_dev, TEST_AREA_OFFSET, EXPECTED_SIZE, &sector_mask);
 
	LOG_DBG("Sector mask for offset 0x%x size 0x%x is 0x%x",
	       TEST_AREA_OFFSET, EXPECTED_SIZE, sector_mask);
  
    test_stm32_write_protection();

    return 0;

}
#endif