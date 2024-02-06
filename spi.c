#include <8051.h>
#include <stdint.h>
#include "periph.h"
#include "constant.h"

/* The Wiznet5100 operates in units of 32-bit streams
    The stream is clocked in MSB first, with the first unit
    being composed of a byte sized OP code: 0x0F and 0XF0 for
    read/write respectively. The next 2 bytes contain and an
    address field with the last byte consisting of the data field.
    
    The W5100 ignores any instruction that doesn't contain either the 0XF0
    or 0X0F op code.*/

// W5100 Write Operation has input 2 byte address field and 1 byte data field
void cmdout(uint16_t addr, uint8_t data) 
{
    uint8_t op = OP_WRITE;

    // clock out op-code signifying a write operation
    for (uint8_t i = 0; i < 8; i++) {
        if (op & MSK_8) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
         // timer delay
        CLK = 1;
        CS = 1;
        op = op << 1; // shift out MSB 
        CLK = 0;

    }

    // write 2 byte address

    // write 1 byte data field
}

// void cmdin(unsigned char cmd) 
// {
//     for (unsigned char i = 0; i < 8; i++) {
//         if (cmd & MSK) {
//             SDA = 1;
//         }
//         else {
//             SDA = 0;
//         }
//         delay(10);
//         SCL = 1;
//         cmd = cmd << 1;  // clock data into sda
//         SCL = 0;
//     }
// }

