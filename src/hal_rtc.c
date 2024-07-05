
#include "hal_rtc.h"

/* Wed Dec 31 2025 23:59:55 GMT+0000 */
#define RTC_TEST_GET_SET_TIME	  (1767225595UL)
#define RTC_TEST_GET_SET_TIME_TOL (1UL)

static const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

void rtc_write_time(time_t timer_set)
{
	struct rtc_time datetime_set;
	gmtime_r(&timer_set, (struct tm *)(&datetime_set));
	rtc_set_time(rtc, &datetime_set);
}

time_t rtc_read_time(struct rtc_time *datetime_get)
{
	time_t timer_get;
	rtc_get_time(rtc, datetime_get);
	timer_get = timeutil_timegm((struct tm *)(datetime_get));
	// printk("datetime_get.tm_sec = %d, %lld\r\n", datetime_get->tm_sec, timer_get);
	return timer_get;
}

#if 0
void rtc_test_loop(void)
{
    rtc_write_time(1767225595UL);

	while (1) {
		struct rtc_time datetime_get;
		time_t timer_get = rtc_read_time(&datetime_get);
		printk("datetime_get.tm_sec = %d, %lld\r\n", datetime_get.tm_sec, timer_get);
        k_msleep(1000);
    }
}
#endif
