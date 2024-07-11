#include <zephyr/drivers/sensor.h>
#include "cpu_temp.h"

#if DT_HAS_COMPAT_STATUS_OKAY(st_stm32_temp_cal)

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cpu_temp, LOG_LEVEL_DBG);

static const struct device *temp_dev = DEVICE_DT_GET(DT_NODELABEL(die_temp));

int cpu_temp_init(void)
{
	if (!device_is_ready(temp_dev)) {
		LOG_WRN("dieTemp device not found!");
        return -ENODEV;
	}
    return 0;
}

int cpu_temp_read(void)
{
    if (!sensor_sample_fetch_chan(temp_dev, SENSOR_CHAN_DIE_TEMP)) {
        struct sensor_value val;
        sensor_channel_get(temp_dev, SENSOR_CHAN_DIE_TEMP, &val);
        float temp_val = sensor_value_to_float(&val) * 10;
        LOG_INF("cpu temp = %d.%d", val.val1, val.val2);
        return (int)temp_val;
    }
    return 0;
}   

#endif
