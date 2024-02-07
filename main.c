#include <8051.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
int buf_counter = 0;
// #include "serial.h"
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
        CLK = HIGH;
        command = command << 1;
        CLK = LOW;
    }
}

// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) 
{
    CS = LOW;
    uint8_t command = OP_WRITE;
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
    cmdout_8(OP_READ);
    cmdout_16(addr);

    // shift in data
    for (uint8_t i = 0; i < 8; i++) {
        CLK = HIGH;
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
        byte = byte << 1; // create room for new bit
    }

    CS = HIGH;
    return byte;
}

// set gateway address
void wiz_set_gateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(GATEWAY_1, a); 
    wiz_write(GATEWAY_2, b); 
    wiz_write(GATEWAY_3, c); 
    wiz_write(GATEWAY_4, d); 
}

// set subnet address
void wiz_set_subnet(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(SUBNET_1, a);
    wiz_write(SUBNET_2, b); 
    wiz_write(SUBNET_3, c); 
    wiz_write(SUBNET_4, d); 
}

// set source mac address
void wiz_set_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) 
{
    wiz_write(MAC_1, a);
    wiz_write(MAC_2, b);
    wiz_write(MAC_3, c);
    wiz_write(MAC_4, d);
    wiz_write(MAC_5, e);
    wiz_write(MAC_6, f);
}

// set ip address
void wiz_set_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(IP_1, a); 
    wiz_write(IP_2, b); 
    wiz_write(IP_3, c); 
    wiz_write(IP_4, d); 
}

// set socket sock_n's port number with inputs upper and lower half of #'s hex value
// 
void wiz_set_port(uint8_t sock_n, uint8_t hex_upper, uint8_t hex_lower) 
{
    
    if (sock_n == 0) {
        wiz_write(SOCKET_0_PORT_U, hex_upper);
        wiz_write(SOCKET_0_PORT_L, hex_lower);
    }
    else if (sock_n == 1) {
        wiz_write(SOCKET_1_PORT_U, hex_upper);
        wiz_write(SOCKET_1_PORT_L, hex_lower);
    }

    
}


// Initialize network
void wiz_init(void) 
{
    // TODO: config mode register
    // TODO: config interrupt mask register
    // TODO: config retry time-value register
    // TODO: config retry count register
    /* Init Sockets 1 and 2 for UDP and TCP*/
    wiz_write(SOCKET_0, 0x02); // set socket 0 to UDP with no multicast
    wiz_write(SOCKET_1, 0x01); // set socket 1 to TCP with ack on internal timeout
    wiz_set_port(0,0x13,0x88); // set socket 0 udp port to 5100 0x1388
    wiz_set_port(1,0x13,0xec);   // set socket 1 tcp port to 5000 0x13ec

    // wiz_read(SOCKET_0); // debug confirm socket
    // wiz_read(SOCKET_1); // debug confirm socket
    
    wiz_set_gateway(126,10,220,254);   // 126.10.220.254 <---gateway address // 7e.a.dc.fe
    wiz_set_subnet(255,255,192,0);    // 255.255.192.0 <--- subnet mask // ff.ff.c0.0
    wiz_set_ip(126,10,200,0);     // 126.10.218.163 <--- my pc // 126.10.218.163 <--- set mcu
    wiz_set_mac(0x00, 0x08, 0xdc, 0x24, 0x4b, 0x7e);
    

    wiz_read(GATEWAY_1); // debug get gateway
    wiz_read(GATEWAY_2);
    wiz_read(GATEWAY_3);
    wiz_read(GATEWAY_4);
}


void main(void)
{
    CLK = LOW; // set clock idle state
    delay_us(1000);
    wiz_init();
	while (1) {
    // RX_data();

    }
}