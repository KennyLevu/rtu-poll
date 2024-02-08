#include <8051.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "periph.h"
#include "constant.h"
#define CS P2_0 // chip select
#define CLK P2_1 // CLK pin
#define MISO P2_2
#define MOSI P2_3

// delay approximately 10us for each count
void delay_us(unsigned int us_count)
 {  
    while(us_count!=0)
      {
         us_count--;
       }
}

// delay approx 10us
void delay10(void) {
    // formula to calculate timer delay: https://www.electronicshub.org/delay-using-8051-timers/
    TH0 = 0xFF;
    TL0 = 0xF6;
    TR0 = 1; // timer 0 start
    while (TF0 == 0); // check overflow condition
    TR0 = 0;    // Stop Timer
    TF0 = 0;   // Clear flag
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

// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) 
{
    CS = LOW;
    uint8_t command = OP_WRITE;
    cmdout_8(OP_WRITE);
    cmdout_16(addr);
    cmdout_8(data);
    delay10();
    CS = HIGH;
}

// OP Read Instruction
uint8_t wiz_read(uint16_t addr) 
{
    CS = LOW;
    uint8_t byte = 0; // hold output data
    cmdout_8(OP_READ);
    cmdout_16(addr);
    // delay_us(2);
    // shift in data
    for (uint8_t i = 0; i < 8; i++) {
        CLK = HIGH;
        // delay10();
        // delay_us(2);
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
        byte = byte << 1; // create room for new bit
    }

    CS = HIGH;
    return byte;
}

// set gateway address a.b.c.d
void wiz_set_gateway(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(GATEWAY_1, a); 
    wiz_write(GATEWAY_2, b); 
    wiz_write(GATEWAY_3, c); 
    wiz_write(GATEWAY_4, d); 
}

// prints gateway address to serial
void wiz_get_gateway(void)
{
    char gateway[8] = {'z','.','z','.','z','.','z','\0'};
    gateway[0] = (char)wiz_read(GATEWAY_1); // returns first portion of address
    gateway[2] = (char)wiz_read(GATEWAY_2);
    gateway[4] = (char)wiz_read(GATEWAY_3);
    gateway[6] = (char)wiz_read(GATEWAY_4);
    // serial_txchar(gateway);
    // serial_txstring(gateway);
    // serial_txchar((char)gateway[0]);
    //     serial_txchar('\n');
    // serial_txchar((char)0xdc);
    //     serial_txchar('\n');
    // serial_txchar((char)wiz_read(GATEWAY_2));
    //     serial_txchar('\n');
    // serial_txchar((char)gateway[2]);
    //     serial_txchar('\n');

    // serial_txchar((char)gateway[4]);
    //     serial_txchar('\n');
    // serial_txchar((char)gateway[6]);
    // serial_txchar('\n');
    // serial_txstring("test");
}

// prints ip addr to serial
void wiz_get_ip(void)
{
    char ipaddr[8] = {'z','.','z','.','z','.','z','\0'};
    ipaddr[0] = (char)wiz_read(IP_1); // returns first portion of address
    ipaddr[2] = (char)wiz_read(IP_2);
    ipaddr[4] = (char)wiz_read(IP_3);
    ipaddr[6] = (char)wiz_read(IP_4);
    // serial_txstring(ipaddr);

}

// prints subnet addr to serial
void wiz_get_subnet(void)
{
    char subnet[8] = {'z','.','z','.','z','.','z','\0'};
    subnet[0] = (char)wiz_read(SUBNET_1); // returns first portion of address
    subnet[2] = (char)wiz_read(SUBNET_2);
    subnet[4] = (char)wiz_read(SUBNET_3);
    subnet[6] = (char)wiz_read(SUBNET_4);
    // serial_txstring(subnet);

}

// prints mac to serial
void wiz_get_mac(void)
{
    char mac[12] = {'z','.','z','.','z','.','z','.','z','.','z','\0'};
    mac[0] = (char)wiz_read(MAC_1); // returns first portion of address
    mac[2] = (char)wiz_read(MAC_2); 
    mac[4] = (char)wiz_read(MAC_3); 
    mac[6] = (char)wiz_read(MAC_4); 
    mac[8] = (char)wiz_read(MAC_5); 
    mac[10] = (char)wiz_read(MAC_6); 
    
}
// set subnet address a.b.c.d
void wiz_set_subnet(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(SUBNET_1, a);
    wiz_write(SUBNET_2, b); 
    wiz_write(SUBNET_3, c); 
    wiz_write(SUBNET_4, d); 
}

// set source mac address a.b.c.d.e.f
void wiz_set_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) 
{
    wiz_write(MAC_1, a);
    wiz_write(MAC_2, b);
    wiz_write(MAC_3, c);
    wiz_write(MAC_4, d);
    wiz_write(MAC_5, e);
    wiz_write(MAC_6, f);
}

// set ip address a.b.c.d
void wiz_set_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) 
{
    wiz_write(IP_1, a); 
    wiz_write(IP_2, b); 
    wiz_write(IP_3, c); 
    wiz_write(IP_4, d); 
}

// set socket sock_n's port number with inputs upper and lower half of #'s hex value
void wiz_set_port(uint8_t sock_n, uint8_t hex_upper, uint8_t hex_lower) 
{
    
    if (sock_n == 0) {
        wiz_write(SOCKET0_PORT_U, hex_upper);
        wiz_write(SOCKET0_PORT_L, hex_lower);
    }
    else if (sock_n == 1) {
        wiz_write(SOCKET1_PORT_U, hex_upper);
        wiz_write(SOCKET1_PORT_L, hex_lower);
    }

    
}


// Initialize network
void wiz_init(void) 
{   
    // TODO: config retry time-value register
    // TODO: config retry count register 
    wiz_write(MODE, 0x00);  // disable indirect bus and pppoe, disable ping block mode
    wiz_write(IMR, 0x01);   // disable sockets 3 and 2, PPPoE interrupts | enable ip conflict and desintation unreachable interrupts

    /* Config Initial Source Network Settings */
    wiz_set_gateway(126,10,220,254);   // 126.10.220.254 <---gateway address // 7e.a.dc.fe
    wiz_set_subnet(255,255,192,0);    // 255.255.192.0 <--- subnet mask // ff.ff.c0.0
    wiz_set_mac(0x00, 0x08, 0xdc, 0x24, 0x4b, 0x5e); // mac address read as hex
    wiz_set_ip(126,10,210,10);     // 126.10.218.163 <--- my pc // 126.10.200.0 7e.a.c8.0<--- set mcu



    /* Init Sockets 0/1
       Socket 0: UDP 5100 | 4KB RX/TX | No Multicast
       Socket 1: TCP 5000 | 4KB RX/TX | Timeout ACK*/
    wiz_write(SOCKET0, 0x02); // set socket 0 to UDP with no multicast
    wiz_write(SOCKET1, 0x01); // set socket 1 to TCP with ack on internal timeout
    /* Divide 8KB of RX memory and 8KB of TX memory over sockets 0 and 1
    Byte representation divided into 4 has 2 bits presenting each socket from greatest to least
    ex. 1100 0000 assigns all memory to socket 3 (11 for 8KB) */
    wiz_write(RX_MEM_SIZE, 0x0A);  // assign 4kb to s0/s1 each  0000 1010 
    wiz_write(TX_MEM_SIZE, 0x0A);   // assign 4kb to s0/s1 each 0000 1010
    /* Bind sockets to port numbers */
    wiz_set_port(0,0x13,0x88); // set socket 0 udp port to 5100 0x1388
    wiz_set_port(1,0x13,0xec);   // set socket 1 tcp port to 5000 0x13ec
    
    // /* Initialize TCP Socket*/
    // while (tcp_open != true) {
    //     wiz_write(SOCKET1_COM, OPEN);
    //     if (wiz_read(SOCKET1_STAT) != SOCK_INIT) {
    //         wiz_write(SOCKET1_COM, CLOSED);
    //         continue;
    //     } 
    //     wiz_write(SOCKET1_COM, LISTEN);
    //     if (wiz_read(SOCKET1_STAT) != SOCK_LISTEN) {
    //         wiz_write(SOCKET1_COM, CLOSED);
    //     } else {
    //         tcp_open = true;
    //     }
    // } 

}

void udp_open(void) 
{
    bool udp_open = false;
    /* Initialize UDP Socket*/
    while (udp_open != true) {
        wiz_write(SOCKET0_COM, OPEN); // open socket
        if (wiz_read(SOCKET0_STAT) != SOCK_UDP) {
            wiz_write(SOCKET0_COM, CLOSED); // socket not initialized, retry
        } else {
            udp_open = true;
        }
    }
}


void udp_rx_helper(void) 
{
    uint8_t rxsizu, rxsizl; // stores upper and lower half of rx register size
    uint16_t rx_size = 0x0000;
    uint16_t rx_offset;
    volatile uint8_t *rx_base = (volatile uint8_t *) SOCKET0_RX_BASE;
    volatile uint8_t *rx_start_addr = NULL;
    volatile uint8_t *header_addr = NULL;
    uint16_t upper_size, left_size; // upper size stores uper size of start address, left stores left size of base addr
    uint8_t peer_ip[4];
    uint16_t peer_port = 0x0000, data_size = 0x0000, rxrd = 0x0000;

    /* Get rx register size by combinning upper and lower half size values */
    rxsizu = wiz_read(SOCKET0_RXSIZU);
    rxsizl = wiz_read(SOCKET0_RXSIZL);
    rx_size = rx_size | rxsizl;
    rx_size = rx_size | (rxsizu << 8);

    /* Calculate offset */
    rxrd = rxrd | (wiz_read(SOCKET0_RXRDU) << 8);
    rxrd = rxrd | wiz_read(SOCKET0_RXRDL);
    rx_offset = rxrd + RXTX_MASK;
    /* Get start (physical) address*/
    rx_start_addr = (rx_base + rx_offset);

    /* Read header information 
        1. Get header address and update start addr
            - If reading header will cause overflow in rx buf, 
              split read operation into two parts to prevent overwriting data
            OR
            - copy header size bytes of start address to header address
        2. get remote information and data size from header 
    */
    if ( (rx_offset + UDP_HEADER_SIZE) > (RXTX_MASK + 1) ) {
        // copy upper_size bytes of start_address to header_add
        upper_size = (RXTX_MASK + 1) - rx_offset;
        memcpy(rx_start_addr, header_addr, UDP_HEADER_SIZE);
        // update header_addr
        header_addr += upper_size;
        // copy left size bytes of RX-BASE to header_addr
        left_size = UDP_HEADER_SIZE - upper_size;
        memcpy(rx_base, header_addr, left_size);
        // update offset
        rx_offset = left_size;
    }
    else {
        // copy header size bytes of start addresss to header addr (copy the header)
        memcpy(rx_start_addr, header_addr, UDP_HEADER_SIZE);
    }
    // update start address 
    rx_start_addr = rx_base + rx_offset;

    /* get remote peer information and receive data size from header*/
    for (int i = 0; i < 4; i++) {
        peer_ip[i] = header_addr[i];
    }
    // get port and size numbers by stitching upper and lower bytes
    peer_port = peer_port | (header_addr[4] << 8);
    peer_port = peer_port | header_addr[5];
    data_size = data_size | (header_addr[6] << 8);
    data_size = data_size | header_addr[7];

}

void udp_rx(void) 
{
    // check the reception interrupt bit is set for socket 0
    if (wiz_read(SOCKET0_IR) & 0x04) {
        udp_rx_helper();
    }
}

void setup(void)
{
    SCON = 0x50;    // Serial Mode 1 8 bit UART with timer1 baud rate
    TMOD = 0x21;    // timer 1 mode 2 8 bit auto reload | timer 0 mode 1 16 bit timer
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 1 means baud is determined by timer 1's overflow rate 
    REN = HIGH;        // serial port initialization
    EA = HIGH;			// enable interrupts
	ES = HIGH;			// enable serial port interrupts
}

void main(void)
{
    delay_us(1000);
    setup();
    CLK = LOW; // set clock idle state
    delay_us(1000);
    wiz_init();
	while (1) {
    // RX_data();

    }
}
