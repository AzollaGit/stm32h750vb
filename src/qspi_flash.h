#ifndef INCLUDE_QSPI_FLASH_H_
#define INCLUDE_QSPI_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

int spi_flash_erase(off_t offset, size_t size);

int spi_flash_read(off_t offset, void *data, size_t len);

int spi_flash_write(off_t offset, const void *data, size_t len);

int spi_flash_init(void);
 
#ifdef __cplusplus
}
#endif

#endif
