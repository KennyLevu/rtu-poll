#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
void delay_us(unsigned int us_count);
void delay10(void);
unsigned char RX_data(void);
// void byte_to_ascii(uint16_t num, char *ascii_str);
void serial_txchar(char ch);
// void serial_ln(void);
// void serial_tab(void); 
void serial_txstring(char *string_ptr);
// void serial_txnum(uint16_t val);
void serial_txhex(uint8_t val);
char *itoa(uint16_t i);


#endif
