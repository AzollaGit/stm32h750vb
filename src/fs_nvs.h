#ifndef INCLUDE_FS_NVS_H_
#define INCLUDE_FS_NVS_H_

#ifdef __cplusplus
extern "C" {
#endif

int fs_nvs_write(uint16_t id, const void *data, size_t len);
 
int fs_nvs_read(uint16_t id, void *data, size_t len);

int fs_nvs_init(void);
 
#ifdef __cplusplus
}
#endif

#endif
