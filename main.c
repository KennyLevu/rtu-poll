#include <8051.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "periph.h"
#include "constant.h"
#include "serial.h"
#include "wiz.h"

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

// Initialize network
void wiz_init(void) 
{   
    wiz_write(MODE, 0x00);  // disable indirect bus and pppoe, disable ping block mode
    wiz_write(IMR, 0xc3);   // disable sockets 3 and 2, PPPoE interrupts | enable ip conflict and desintation unreachable interrupts, 0 and 1
    wiz_write(RETRY_U, 0x07); // timeout periods 200ms
    wiz_write(RETRY_L, 0xd0);
    wiz_write(RETRY_COUNT, 5);

    /* Config Initial Source Network Settings */
    wiz_set_gateway(126,10,220,254);   // 126.10.220.254 <---gateway address // 7e.a.dc.fe
    wiz_set_subnet(255,255,192,0);    // 255.255.192.0 <--- subnet mask // ff.ff.c0.0
    wiz_set_mac(0x00, 0x08, 0xdc, 0x24, 0x4b, 0x5e); // mac address read as hex
    wiz_set_ip(126,10,210,10);     // 126.10.218.163 <--- my pc // 126.10.200.0 7e.a.c8.0<--- set mcu



    /* Init Sockets 0/1
       Socket 0: UDP 5000 | 4KB RX/TX | No Multicast
       Socket 1: TCP 5100 | 4KB RX/TX | Timeout ACK*/
    wiz_write(SOCKET0, 0x02); // set socket 0 to UDP with no multicast
    // wiz_write(SOCKET1, 0x01); // set socket 1 to TCP with ack on internal timeout
    /* Divide 8KB of RX memory and 8KB of TX memory over sockets 0 and 1
    Byte representation divided into 4 has 2 bits presenting each socket from greatest to least
    ex. 1100 0000 assigns all memory to socket 3 (11 for 8KB) */
    wiz_write(RX_MEM_SIZE, 0x0A);  // assign 4kb to s0/s1 each  0000 1010 
    wiz_write(TX_MEM_SIZE, 0x0A);   // assign 4kb to s0/s1 each 0000 1010
    /* Bind sockets to port numbers */
    wiz_set_port(0,0x13,0x88); // set socket 0 udp port to 5000 0x1388
    wiz_set_port(1,0x13,0xec);   // set socket 1 tcp port to 5100 0x13ec

    
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
    while (udp_open == false) {
        // serial_txstring("still trying");
        wiz_write(SOCKET0_COM, OPEN); // open socket
        if (wiz_read(SOCKET0_STAT) != SOCK_UDP) {
            wiz_write(SOCKET0_COM, CLOSED); // socket not initialized, retry
        } else {
            serial_txstring("UDP Socket 0 port 5000 open\n");
            udp_open = true;
        }
    }
}




void udp_rx_helper(void) 
{
    serial_txstring("UDP Packet Received\n");
    uint16_t rx_offset, rx_start_addr, upper_size, left_size; // upper size stores uper size of start address, left stores left size of base addr
    uint8_t rxsizu, rxsizl; // stores upper and lower half of rx register size
    uint16_t peer_port = 0x0000, data_size = 0x0000, rxrd = 0x0000;
    uint16_t rx_base = SOCKET0_RX_BASE; // pointer to base address of rx register
    uint16_t rx_size = 0x0000;
    uint8_t buf_header[8] = {0};
    uint8_t peer_ip[4] = {0};
    // uint8_t *peer_data = NULL;

    /* Get rx register size by combinning upper and lower half size values */
    rxsizu = wiz_read(SOCKET0_RXSIZU);
    rxsizl = wiz_read(SOCKET0_RXSIZL);
    rx_size = rx_size | rxsizl;
    rx_size = rx_size | (rxsizu << 8);
    // serial_txstring("RX Size: ");
    // serial_txnum(rx_size);
    

    /* Calculate offset */

    rxrd = rxrd | (wiz_read(SOCKET0_RXRDU) << 8);
    rxrd = rxrd | wiz_read(SOCKET0_RXRDL);
    // serial_txstring("\t rxrd: ");
    // serial_txnum(rxrd);
    // serial_txstring("\t rxrd: ");
    // serial_txnum(rxrd);

    // serial_txstring("\t rxrd: ");
    // serial_txnum(rxrd);

    // serial_txstring("\t Mask: ");
    // serial_txnum(RXTX_MASK);
    rx_offset = rxrd & RXTX_MASK;

    // serial_txstring("\t Offset: ");
    // serial_txnum(RXTX_MASK);
    
    /* Get start (physical) address*/
    rx_start_addr = SOCKET0_RX_BASE + rx_offset;
    // serial_txstring("\t start_addr: ");
    // serial_txnum(rx_start_addr);

    /* Read header information 
        1. Get header address and update start addr
            - If reading header will cause overflow in rx buf, 
              split read operation into two parts to prevent overwriting data

              **** "There's a case that it exceeds the RX memory upper-bound of the socket while reading. In this case, read
                    the receiving data to the upper-bound, and change the physical address to the gSn_RX_BASE.
                    Next, read the rest of the receiving data." ****

            OR
            - copy header size bytes of start address to header address
        2. get remote information and data size from header 
    */
    if ( (rx_offset + UDP_HEADER_SIZE) > (RXTX_MASK + 1) ) {
        serial_txstring("\nUDP RX Header Overflow Detected\n");
        upper_size = (RXTX_MASK + 1) - rx_offset; // get difference between end of buffer and offset
        wiz_read_buf(rx_start_addr, upper_size, buf_header); 
        left_size = UDP_HEADER_SIZE - upper_size; // get the remaining amount of data 
        wiz_read_buf(rx_base, left_size, buf_header + (upper_size - 1)); // read from overflow point base of rx
        // update offset past header
        rx_offset = left_size;
    }
    else {
        // copy header size bytes of start addresss to header addr (copy the header)
        wiz_read_buf(rx_start_addr, UDP_HEADER_SIZE, buf_header);
        // update offset past header
        rx_offset += UDP_HEADER_SIZE;
    }
    // update start address 
    rx_start_addr = SOCKET0_RX_BASE + rx_offset;

    /* get remote peer information and receive data size from header*/
    // get port and size numbers by stitching upper and lower bytes
    peer_port = peer_port | (buf_header[4] << 8);
    peer_port = peer_port | buf_header[5];
    data_size = data_size | (buf_header[6] << 8);
    data_size = data_size | buf_header[7];

    serial_txstring("Sender IP: ");
    for (int i = 0; i < 4; i++) {
        peer_ip[i] = buf_header[i];
        serial_txnum(buf_header[i]);
        if (i < 3) {
            serial_txchar('.');
        }
    }
    serial_tab();
    serial_txstring("Sender Port: ");
    serial_txnum(peer_port);
    serial_tab();
    serial_txstring("Data Size: ");
    serial_txnum(data_size);
    serial_ln();

    // Allocate buffer for data size
    // peer_data = malloc(sizeof(uint8_t) * data_size);


    /* Read Data
        1. Check if data size overflows rx buffer
            - Read data in two parts
    */
    // if( (rx_offset + data_size) > (RXTX_MASK + 1) ) {
    //     serial_txstring("\nUDP RX DATA Overflow Detected\n");
    //     upper_size = (RXTX_MASK + 1) - rx_offset; // get first part of data
    //     wiz_read_buf(rx_start_addr, upper_size, peer_data);
    //     left_size = data_size - upper_size; // get remaining data 
    //     wiz_read_buf(rx_start_addr, left_size, peer_data + upper_size); // read from overflow point
    // }
    // else {
    //     wiz_read_buf(rx_start_addr, data_size, peer_data);
    // }
    // for (int i = 0; i < data_size-UDP_HEADER_SIZE; i++) {
    //     serial_txchar(peer_data[i]);
    //     serial_ln();
    // }
    /* increase Sn_RX_RD as length of data_size+header_size */
    rxrd += data_size;
    rxrd += UDP_HEADER_SIZE;

    serial_txstring("Computed rxrd: ");
    serial_txnum(rxrd);
    serial_ln();
    wiz_write(SUBNET_1, 10);
    serial_txstring("subnet: ");
    serial_txreg(SUBNET_1);
    serial_txstring("rx reg size start:");
    serial_tx2reg(SOCKET0_RXSIZU, SOCKET0_RXSIZL);
    serial_txstring("rxrd pointer start: ");
    serial_tx2reg(SOCKET0_RXRDU, SOCKET0_RXRDL);
    // RXRD is processed with received command
    uint8_t upper = (rxrd >> 8) & 0xff;
    uint8_t lower =  (rxrd&0xff);
    serial_txstring("upper:");
    serial_txnum(upper);
    serial_txstring("lower:");
    serial_txnum(lower);
    serial_ln();
    // wiz_write(SOCKET0_RXRDU, upper);
    // wiz_write(SOCKET0_RXRDL, 1);
    serial_txstring("direct rxrd read: ");
    serial_txnum(wiz_read(SOCKET0_RXRDU));
    serial_txnum(wiz_read(SOCKET0_RXRDL));
    serial_txstring("rxrd pointer end: ");
    serial_tx2reg(SOCKET0_RXRDU, SOCKET0_RXRDL);
    // Set received command
    wiz_write(SOCKET0_COM, RECV);
    serial_txstring("rx reg size:");
    serial_tx2reg(SOCKET0_RXSIZU, SOCKET0_RXSIZL);
    serial_txstring("Interrupt reg: ");
    serial_txreg(SOCKET0_IR);
    // free(peer_data);
}

void udp_rx(void) 
{
    if (wiz_read(SOCKET0_IR) & 0x04) { // check for Recv interrupt (bit 2/ 100 / x04)
        // clear interrupt
        wiz_write(SOCKET0_IR, 1);
        udp_rx_helper();
    }
}

void setup(void)
{
    SCON = 0x50;    // Serial Mode 1 8 bit UART with timer1 baud rate
    TMOD = 0x20;    // timer 1 mode 2 8 bit auto reload | timer 0 mode 1 16 bit timer
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 1 means baud is determined by timer 1's overflow rate 
    REN = HIGH;        // serial port initialization
    EA = HIGH;			// enable interrupts
	ES = HIGH;			// enable serial port interrupts
    TR1 = 1; // start timer
}

void main(void)
{
    // delay_us(1000);
    wiz_write(SOCKET0_RXRDU, 0xfe);
        wiz_write(SOCKET0_RXRDL, 0xd);
    setup();
    CLK = LOW; // set clock idle state
    // delay_us(1000);
    
    wiz_init();
    // udp_open();
	while (1) {
        wiz_write(SOCKET0_PORT_L, 0xfe);
        wiz_read(SOCKET0_PORT_L);
        // wiz_write(SOCKET0_RXRDU, 0xfe);
        // wiz_write(SOCKET0_RXRDL, 0xd);
        wiz_read(SOCKET0_RXRDU);
        wiz_read(SOCKET0_RXRDL);
        // serial_tx2reg(SOCKET0_RXRDU, SOCKET0_RXRDL);
        // udp_rx();
    }
}
