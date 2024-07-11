
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>

#include "hal_pwm.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hal_pwm, LOG_LEVEL_DBG);

static const struct pwm_dt_spec pwm0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

// pwm_led0 => tim12_ch1_pb14
void hal_pwm_setup(void)
{
    if (!pwm_is_ready_dt(&pwm0)) {
		LOG_WRN("Error: PWM device %s is not ready", pwm0.dev->name);
		return;
	}
	LOG_DBG("pwm0.period = %d", pwm0.period);
}


void hal_pwm_set_duty(uint8_t duty)
{   
    uint32_t pulse = (pwm0.period / 100) * duty;
    int ret = pwm_set_pulse_dt(&pwm0, pulse);
    if (ret) {
        LOG_WRN("failed pwm_set_dt, Error: %d", ret);
        return;
    }
}	