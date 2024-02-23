#include <8051.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "periph.h"
#include "constant.h"

// delay approximately 10us for each count
void delay_us(unsigned int us_count)
 {  
    while(us_count!=0)
      {
         us_count--;
         us_count--;
       }
}

void serial_txchar(char ch)
{
    SBUF=ch;       // Load the data to be transmitted into serial buffer
    while(TI == 0) {
                // TI is set when SBUF is transmitted, wait for TI to change
    }   
    TI = 0;    // clear the flag for next byte

}
void serial_txstring(char *string_ptr)
{

          while(*string_ptr) {         // iterate over string buffer until null 
            SBUF=*string_ptr++;       // load character byte into SBUF and increment to the next character
            while(TI == 0) {
                // TI is set when SBUF is transmitted, wait for TI to change
            }    
            TI = 0;    // clear the flag for next byte
          }
}

// converts byte to 1 byte hex representation with a bit mask and digit dictionary
void byte_to_hex(uint8_t byte, char* hex) {
    const char* hex_digits = "0123456789ABCDEF"; // dictionary for hex digits maps index to corresponding hex value
    hex[0] = hex_digits[(byte >> 4) & 0x0F]; // store lower half of byte in hex format by masking lower half of input byte
    hex[1] = hex_digits[byte & 0x0F]; // store upper half
    hex[2] = '\0';
}

void serial_txhex(uint8_t val) {
    char str[3] = {'\0'};
    byte_to_hex(val, str);
    serial_txstring(str);
}


// https://opensource.apple.com/source/groff/groff-10/groff/libgroff/itoa.c
char *itoa(uint16_t i) 
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[5 + 1]; // 5 + 2 --> 5 + 1 for no negatives, num in buf being the number of digits + the null terminator
  char *p = buf + 5;	/* points to terminating '\0' */
//   if (i >= 0) {
    do { 
      *--p = '0' + (i % 10); // move to next digit right to left by one, then convert int into ascii by isolating the digit at pointer
      i /= 10; // slice off right most digit in the ones place to check the next digit
    } while (i != 0); // end loop when all digits are reached
    return p; // return ascii buffer
//   }
//   else {			/* i < 0 */
//     do {
//       *--p = '0' - (i % 10);
//       i /= 10;
//     } while (i != 0);
//     *--p = '-';
//   }
//   return p;
}

unsigned char RX_data(void)
{
    if (RI == 1) { // RX interrupt bit is set
        unsigned char a; // initialize char to hold byte character
        RI = 0; // clear interrupt bit
        a = SBUF; // store char character from buffer
        return a;
    }
    return 0;
}