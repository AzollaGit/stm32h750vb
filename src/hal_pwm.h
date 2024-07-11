#ifndef INCLUDE_HAL_PWM_H_
#define INCLUDE_HAL_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif
 
void hal_pwm_setup(void);
void hal_pwm_set_duty(uint8_t duty);
 
#ifdef __cplusplus
}
#endif

#endif
