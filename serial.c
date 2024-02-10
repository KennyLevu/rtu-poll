#include <8051.h>
#include <stdint.h>
#include "periph.h"
#include "constant.h"
#include "wiz.h"

// char getCharacter (void)
// {
//     char chr;
//     while (RI != 1) {;}

//     while(SBUF != '~')
//     {
//     chr = chr + SBUF;
//     }
//     RI = 0;
//     return(chr);
// }



void byte_to_ascii(uint16_t num, char *ascii_str) 
{
    // Convert each digit to its ASCII representation
    if (num >= 10000) {
        ascii_str[0] = '0' + (num / 10000);  // Ten thousands place
        ascii_str[1] = '0' + ((num / 1000) % 10);  // Thousands place
        ascii_str[2] = '0' + ((num / 100) % 10);  // Hundreds place
        ascii_str[3] = '0' + ((num / 10) % 10);  // Tens place
        ascii_str[4] = '0' + (num % 10);  // Ones place
        ascii_str[5] = '\0';  // Null-terminate the string
    }
     else if (num >= 1000) {
        ascii_str[0] = '0' + (num / 1000);
        ascii_str[1] = '0' + ((num / 100) % 10);
        ascii_str[2] = '0' + ((num / 10) % 10);
        ascii_str[3] = '0' + (num % 10);
        ascii_str[4] = '\0';
    }
    else if (num >= 100) {
        ascii_str[0] = '0' + (num / 100);
        ascii_str[1] = '0' + ((num / 10) % 10);
        ascii_str[2] = '0' + (num % 10);
        ascii_str[3] = '\0';
    } else if (num >= 10) {
        ascii_str[0] = '0' + (num / 10);
        ascii_str[1] = '0' + (num % 10);
        ascii_str[2] = '\0';
    } else {
        ascii_str[0] = '0' + num;
        ascii_str[1] = '\0';
    }
}
void serial_txchar(char ch)
{
    SBUF=ch;       // Load the data to be transmitted
    while(TI == 0);    // Wait till the data is trasmitted
        TI = 0;         //Clear the flag for next cycle.
}

void serial_ln(void) 
{
    SBUF='\n';
    while(TI == 0);
        TI = 0;   
}
void serial_tab(void) 
{
    SBUF='\t';
    while(TI == 0);
        TI = 0;   
}

void serial_txstring(char *string_ptr)
{
          while(*string_ptr)
           serial_txchar(*string_ptr++);
}

void serial_txnum(uint16_t val)
{
    char str[6] = {'\0'};
    byte_to_ascii(val, str);
    serial_txstring(str);
}

// void serial_txreg(uint16_t addr)
// {
//     serial_txnum(wiz_read(addr));
//     serial_ln();
// }

// void serial_tx2reg(uint16_t upper, uint16_t lower)
// {
//     uint16_t combined = 0x0000; 
//     uint8_t up = wiz_read(upper);
//     uint8_t lo = wiz_read(lower);
//     combined = combined | lo;
//     combined = combined | (up << 8);
//     serial_txnum(combined);
//     serial_ln();
// }

unsigned char RX_data(void)
{
    unsigned char a;
    while(RI == 0);
    RI = 0;
    a = SBUF;
    serial_txchar(a);
    return a;
}