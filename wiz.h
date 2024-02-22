#ifndef WIZ_H
#define WIZ_H
#include <stdint.h>
void wiz_write(uint16_t addr, uint8_t data);
uint8_t wiz_read(uint16_t addr);
void wiz_read_buf(uint16_t addr, uint16_t len, uint8_t *buffer);
void wiz_write_buf(uint16_t addr, uint16_t len, uint8_t *buffer);

#endif