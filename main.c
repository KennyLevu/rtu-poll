#include <8051.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
int buf_counter = 0;
// #include "serial.h"
#include "periph.h"
#include "constant.h"

// Initial configuartion setting and code sourced from: https://exploreembedded.com/wiki/8051_Family_C_Library


void serial_init(void)
{
    // SCON = SCON & 0x0F; 
    SCON = 0x50;    // serial mode 3 8 bit data 1 start and 1 stop bit
    TMOD = 0x20;    // timer 1 mode 2 and timer 2 mode 0 ---> mode 2 is 16bit timer/counter auto reload and thx/tlx are cascaded
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 3 means baud is determined by timer 1's overflow rate 
    REN = HIGH;        // serial port initialization
    EA = HIGH;			// enable interrupts
	ES = HIGH;			// enable serial port interrupts
    TR1 = HIGH;        // Turn ON timer 1

}


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

void serial_txchar(char ch)
{
    SBUF=ch;       // Load the data to be transmitted
    while(TI==0);    // Wait till the data is trasmitted
        TI=0;         //Clear the flag for next cycle.
}

void serial_txstring(char *string_ptr)
 {
          while(*string_ptr)
           serial_txchar(*string_ptr++);
}

unsigned char RX_data(void)
{
    unsigned char a;
    while(RI == 0);
    RI = 0;
    a = SBUF;
    serial_txchar(a);
    return a;
}


// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) {
    CS = LOW;
    uint8_t command = OP_WRITE;
    // send op code
    for (uint8_t i = 0; i < 8; i++) {
        if (command & MSK_8) {
            MOSI = HIGH;
        }
        else {
            MOSI = 0;
        }
        CLK = HIGH;
        command = command << 1;
        CLK = LOW;
    }

    // send address field
    for (uint8_t i = 0; i < 16; i++) {
        if (addr & MSK_16) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        CLK = HIGH;
        addr = addr << 1;
        CLK = LOW;
    }

    // send data
    for (uint8_t i = 0; i < 8; i++) {
        if (data & MSK_8) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        CLK = HIGH;
        data = data << 1;
        CLK = LOW;
    }

    CS = HIGH;
}

// OP Read Instruction
uint8_t wiz_read(uint16_t addr) {
    CS = LOW;
    uint8_t command = OP_READ;
    uint8_t byte = 0; // hold output data
    // send op code
    for (uint8_t i = 0; i < 8; i++) {
        if (command & MSK_8) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        CLK = HIGH;
        command = command << 1;
        CLK = LOW;
    }

    // send address field
    for (uint8_t i = 0; i < 16; i++) {
        if (addr & MSK_16) {
            MOSI = HIGH;
        }
        else {
            MOSI = LOW;
        }
        CLK = HIGH;
        addr = addr << 1;
        CLK = LOW;
    }

    // shift in data
    for (uint8_t i = 0; i < 8; i++) {
        byte = byte << 1; // create room for new bit
        CLK = HIGH;
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
    }

    CS = HIGH;
    return byte;
}

// set gateway address
void wiz_gateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    
}

// set subnet address
void wiz_subnet(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    
}

// Initialize network
void wiz_init(void) {
    // TODO: config mode register
    // TODO: config interrupt mask register
    // TODO: config retry time-value register
    // TODO: config retry count register

    /* Init Sockets 1 and 2 for UDP and TCP*/
    wiz_write(SOCKET_0, 0x02); // set to UDP with no multicast
    wiz_write(SOCKET_1, 0x01); // set to TCP with ack on internal timeout
    // wiz_read(SOCKET_0); // debug confirm socket
    // wiz_read(SOCKET_1); // debug confirm socket

    // 126.10.220.254
    wiz_write(GATEWAY_1, 126); //7e
    wiz_write(GATEWAY_2, 10); // a
    wiz_write(GATEWAY_3, 220); // dc
    wiz_write(GATEWAY_4, 254); // fe
    // wiz_read(GATEWAY_1); // debug get gateway
    // wiz_read(GATEWAY_2);
    // wiz_read(GATEWAY_3);
    // wiz_read(GATEWAY_4);

    // 255.255.192.0 <--- subnet mask
    wiz_write(SUBNET_1, 255);
    wiz_write(SUBNET_2, 255);
    wiz_write(SUBNET_3, 192);
    wiz_write(SUBNET_4, 0);
}

void main(void)
{
    CLK = LOW; // set clock idle state
    // MISO = LOW;
    wiz_init();
	while (1) {
    // RX_data();

    }
}