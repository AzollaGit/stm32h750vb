#ifndef INCLUDE_CPU_TEMP_H_
#define INCLUDE_CPU_TEMP_H_

#ifdef __cplusplus
extern "C" {
#endif

int cpu_temp_init(void);

int cpu_temp_read(void);
 
#ifdef __cplusplus
}
#endif

#endif
