#include <8051.h>
#include <stdint.h>
#include "periph.h"
#include "constant.h"

// write bytes to MOSI 
void cmdout_16(uint16_t command) 
{
    for (uint8_t i = 0; i < BYTE_16; i++) {
        if (command & MSK_16) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        // delay10();
        CLK = HIGH;
        command = command << 1;
        CLK = LOW;
    }
}
void cmdout_8(uint8_t command) 
{
    for (uint8_t i = 0; i < BYTE_8; i++) {
        if (command & MSK_8) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        // delay10();
        CLK = HIGH;
        command = command << 1;
        CLK = LOW;
    }
}

// void asmout_8(uint8_t command) 
// {
//     uint8_t i;
//     for (i = 0; i < 8; i++) {
//         asm volatile ( // tells compiler that the following is assembly code, volatile flags the code to prevent opimization 
//             "mov     r1, %[cmd]       \n"  // Move the command to a register
//             "srl     r1, #7           \n"  // Shift the command to get the most significant bit
//             "jc      set_bit          \n"  // If carry flag is set, jump to set_bit label
//             "clr     P2.3             \n"  // Clear P2.3 if the MSB is 0
//             "sjmp    next_bit         \n"  // Jump to next_bit label
//             "set_bit:                 \n"  // Label to set P2.3 if MSB is 1
//             "setb    P2.3             \n"  // Set P2.3 if the MSB is 1
//             "next_bit:                \n"  // Label to proceed to the next bit
//             "setb    P2.1             \n"  // Set P2.1
//             "clr     P2.1             \n"  // Clear P2.1
//             : // No output operands
//             : [cmd] "r" (command) // Input operand - command
//             : "r1", "cc" // clobber list - register r1 and condition codes
//         );
//         command = command << 1; // Shift the command to the left by one bit
//     }
// }
// void asm_16(uint16_t command) 
// {
//     uint8_t i;
//     for (i = 0; i < 16; i++) {
//         asm volatile ( 
//             "mov     r1, %[cmd]       \n" 
//             "srl     r1, #15          \n"
//             "jc      set_bit          \n"
//             "clr     P2.3             \n"
//             "sjmp    next_bit         \n"
//             "set_bit:                 \n"
//             "setb    P2.3             \n"
//             "next_bit:                \n"
//             "setb    P2.1             \n"
//             "clr     P2.1             \n"
//             :
//             : [cmd] "r" (command)
//             : "r1", "cc"
//         );
//         command = command << 1;
//     }
// }

// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) 
{
    CS = LOW;
    // uint8_t command = OP_WRITE;
    cmdout_8(OP_WRITE);

    cmdout_16(addr);

    cmdout_8(data);
    CS = HIGH;
}

// OP Read Instruction
uint8_t wiz_read(uint16_t addr) 
{
    CS = LOW;
    uint8_t byte = 0; // hold output data
    // uint8_t command = OP_READ;
    cmdout_8(OP_READ);

    cmdout_16(addr);

    for (uint8_t i = 0; i < 8; i++) {
        CLK = HIGH;
        // delay10();
        // delay_us(2);
        byte = byte << 1; // create room for new bit
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
    }

    CS = HIGH;
    return byte;
}


// set gateway address a.b.c.d
// void wiz_set_gateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
// {
//     wiz_write(GATEWAY_1, a); 
//     wiz_write(GATEWAY_2, b); 
//     wiz_write(GATEWAY_3, c); 
//     wiz_write(GATEWAY_4, d); 
// }

// prints gateway address to serial
// void wiz_get_gateway(void)
// {
//     char gateway[8] = {'z','.','z','.','z','.','z','\0'};
//     gateway[0] = (char)wiz_read(GATEWAY_1); // returns first portion of address
//     gateway[2] = (char)wiz_read(GATEWAY_2);
//     gateway[4] = (char)wiz_read(GATEWAY_3);
//     gateway[6] = (char)wiz_read(GATEWAY_4);
// }

// prints ip addr to serial
// void wiz_get_ip(void)
// {
//     char ipaddr[8] = {'z','.','z','.','z','.','z','\0'};
//     ipaddr[0] = (char)wiz_read(IP_1); // returns first portion of address
//     ipaddr[2] = (char)wiz_read(IP_2);
//     ipaddr[4] = (char)wiz_read(IP_3);
//     ipaddr[6] = (char)wiz_read(IP_4);
//     // serial_txstring(ipaddr);

// }

// // prints subnet addr to serial
// void wiz_get_subnet(void)
// {
//     char subnet[8] = {'z','.','z','.','z','.','z','\0'};
//     subnet[0] = (char)wiz_read(SUBNET_1); // returns first portion of address
//     subnet[2] = (char)wiz_read(SUBNET_2);
//     subnet[4] = (char)wiz_read(SUBNET_3);
//     subnet[6] = (char)wiz_read(SUBNET_4);
//     // serial_txstring(subnet);

// }

// prints mac to serial
// void wiz_get_mac(void)
// {
//     char mac[12] = {'z','.','z','.','z','.','z','.','z','.','z','\0'};
//     mac[0] = (char)wiz_read(MAC_1); // returns first portion of address
//     mac[2] = (char)wiz_read(MAC_2); 
//     mac[4] = (char)wiz_read(MAC_3); 
//     mac[6] = (char)wiz_read(MAC_4); 
//     mac[8] = (char)wiz_read(MAC_5); 
//     mac[10] = (char)wiz_read(MAC_6); 
    
// }
// // set subnet address a.b.c.d
// void wiz_set_subnet(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
// {
//     wiz_write(SUBNET_1, a);
//     wiz_write(SUBNET_2, b); 
//     wiz_write(SUBNET_3, c); 
//     wiz_write(SUBNET_4, d); 
// }

// // set source mac address a.b.c.d.e.f
// void wiz_set_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) 
// {
//     wiz_write(MAC_1, a);
//     wiz_write(MAC_2, b);
//     wiz_write(MAC_3, c);
//     wiz_write(MAC_4, d);
//     wiz_write(MAC_5, e);
//     wiz_write(MAC_6, f);
// }

// // set ip address a.b.c.d
// void wiz_set_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
// {
//     wiz_write(IP_1, a); 
//     wiz_write(IP_2, b); 
//     wiz_write(IP_3, c); 
//     wiz_write(IP_4, d); 
// }

// // set socket sock_n's port number with inputs upper and lower half of #'s hex value
// void wiz_set_port(uint8_t sock_n, uint8_t hex_upper, uint8_t hex_lower) 
// {
    
//     if (sock_n == 0) {
//         wiz_write(SOCKET0_PORT_U, hex_upper);
//         wiz_write(SOCKET0_PORT_L, hex_lower);
//     }
//     else if (sock_n == 1) {
//         wiz_write(SOCKET1_PORT_U, hex_upper);
//         wiz_write(SOCKET1_PORT_L, hex_lower);
//     }

    
// }

// Read data buffer from wiznet address
void wiz_read_buf(uint16_t addr, uint16_t len, uint8_t *buffer) 
{
    for (int i = 0; i < len; i++) {
        buffer[i] = wiz_read(addr + i);
    }
}

// write to address from data buffer
void wiz_write_buf(uint16_t addr, uint16_t len, uint8_t *buffer) 
{
    for (int i = 0; i < len; i++) {
        wiz_write(addr + i, buffer[i]);
    }
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
