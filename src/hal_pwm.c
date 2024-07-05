
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>

#include "hal_pwm.h"

static const struct pwm_dt_spec pwm0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

// pwm_led0 => tim12_ch1_pb14
void hal_pwm_setup(void)
{
    if (!pwm_is_ready_dt(&pwm0)) {
		printk("Error: PWM device %s is not ready\n", pwm0.dev->name);
		return;
	}
	printk("pwm0.period = %d\n", pwm0.period);
}

void hal_pwm_set_duty(uint32_t period, uint32_t pulse)
{
    //period = pwm0.period/2;  // 20000000ns / 2 = 10ms
    int ret = pwm_set_dt(&pwm0, period, period / 2U);
    if (ret) {
        printk("Error %d: failed to set pulse width\n", ret);
        return;
    }
}	