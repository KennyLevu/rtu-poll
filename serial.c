#include <8051.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "periph.h"
#include "constant.h"

// delay approximately 10us for each count
void delay_us(unsigned int us_count)
 {  
    while(us_count!=0)
      {
         us_count--;
       }
}

// delay approx 10us
void delay10(void) 
{
    // formula to calculate timer delay: https://www.electronicshub.org/delay-using-8051-timers/
    TH0 = 0xFF;
    TL0 = 0xF6;
    TR0 = 1; // timer 0 start
    while (TF0 == 0); // check overflow condition
    TR0 = 0;    // Stop Timer
    TF0 = 0;   // Clear flag
}
void serial_txchar(char ch)
{
    SBUF=ch;       // Load the data to be transmitted
    while(TI == 0);    // Wait till the data is trasmitted
        TI = 0;         //Clear the flag for next cycle.
}
void serial_txstring(char *string_ptr)
{
    // while(string_ptr != '\0') {
    //     serial_txchar(*string_ptr);
    //     string_ptr++;
    // }
    // \r\nCOM\0

          while(*string_ptr) {
            serial_txchar(*string_ptr++);
          }
}

// converts byte to 1 byte hex representation with a bit mask and digit dictionary
void byte_to_hex(uint8_t byte, char* hex) {
    const char* hex_digits = "0123456789ABCDEF"; // dictionary for hex digits maps index to corresponding hex value
    hex[0] = hex_digits[(byte >> 4) & 0x0F];
    hex[1] = hex_digits[byte & 0x0F];
    hex[2] = '\0';
}

void serial_txhex(uint8_t val) {
    char str[3] = {'\0'};
    byte_to_hex(val, str);
    serial_txstring(str);
}


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
    if (RI == 1) {
        unsigned char a;
        RI = 0;
        a = SBUF;
        // serial_txchar(a);
        return a;
    }
    return 0;
}