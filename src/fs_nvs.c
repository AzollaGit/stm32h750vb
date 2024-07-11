#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nvs_flash, LOG_LEVEL_DBG);
 
#define NVS_PARTITION		    nvs_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define NVS_PARTITION_SIZE  	FIXED_PARTITION_SIZE(NVS_PARTITION)

static struct nvs_fs fs;

// note: len = 0; is delete
int fs_nvs_write(uint16_t id, const void *data, size_t len)
{
	return nvs_write(&fs, id, data, len);
}

int fs_nvs_read(uint16_t id, void *data, size_t len)
{
	return nvs_read(&fs, id, data, len);
}

int fs_nvs_init(void)
{
    int ret;
    struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		LOG_WRN("Flash device %s is not ready", fs.flash_device->name);
		return -ENODEV;
	}
 
	fs.offset = NVS_PARTITION_OFFSET;
	ret = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (ret) {
		LOG_WRN("Unable to get page info");
		return 0;
	}
	fs.sector_size = info.size;
	fs.sector_count = 3U;

	ret = nvs_mount(&fs);
	if (ret) {
		LOG_WRN("Flash Init failed");
		return 0;
	}

    LOG_DBG("nvs_partition@ 0x%08x, size: 0x%x", NVS_PARTITION_OFFSET, NVS_PARTITION_SIZE);
	LOG_DBG("sector_size = %d", fs.sector_size);

#define RBT_CNT_ID  1
#if RBT_CNT_ID  // test...
    uint32_t reboot_counter = 520U;
    ret = nvs_write(&fs, RBT_CNT_ID, &reboot_counter, sizeof(reboot_counter));
	LOG_DBG("nvs_write =  %d", reboot_counter);
    ret = nvs_read(&fs, RBT_CNT_ID, &reboot_counter, sizeof(reboot_counter));
	if (ret > 0) { /* item was found, show it */
		LOG_DBG("Id: %d, Reboot_counter: %d", RBT_CNT_ID, reboot_counter);
	} else   {/* item was not found, add it */
		LOG_WRN("No Reboot counter found, adding it at id %d", RBT_CNT_ID);
	}
#endif
    return 0;
}