#ifndef INCLUDE_HAL_RTC_H_
#define INCLUDE_HAL_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/timeutil.h>
#include <time.h>
#include <string.h>
 
void rtc_write_time(time_t timer_set);
time_t rtc_read_time(struct rtc_time *datetime_get);

#ifdef __cplusplus
}
#endif

#endif