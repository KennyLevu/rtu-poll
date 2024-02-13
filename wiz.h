#ifndef WIZ_H
#define WIZ_H
#include <stdint.h>
// void serial_txreg(uint16_t addr);
// void serial_tx2reg(uint16_t upper, uint16_t lower);
void cmdout_16(uint16_t command);
void cmdout_8(uint8_t command);
void wiz_write(uint16_t addr, uint8_t data);
uint8_t wiz_read(uint16_t addr);
// void wiz_set_gateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void wiz_get_gateway(void);
void wiz_get_ip(void);
void wiz_get_subnet(void);
void wiz_get_mac(void);
// void wiz_set_subnet(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
// void wiz_set_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);
// void wiz_set_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
// void wiz_set_port(uint8_t sock_n, uint8_t hex_upper, uint8_t hex_lower);
void wiz_read_buf(uint16_t addr, uint16_t len, uint8_t *buffer);
void wiz_write_buf(uint16_t addr, uint16_t len, uint8_t *buffer);

#endif