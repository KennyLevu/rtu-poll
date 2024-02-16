#include <8051.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sdcc-lib.h>
#include <ctype.h>
#include "periph.h"
#include "constant.h"
#include "serial.h"
#include "wiz.h"

char rtu[2] = "0";
uint8_t serial_in[22] = {'\0'};
uint8_t serial_pt = 0;
uint8_t addr[8] = {0};
uint8_t server_state = UDP;
static const char com_rtu[5] = "RTU=";
static const char com_ip[4] = "IP=";
static const char com_sub[5] = "SUB=";
static const char com_gate[6] = "GATE=";
static const char com_mac[5] = "MAC=";
static const char com_mode[6] = "MODE=";
uint8_t *peer_data = NULL;


void setup(void)
{
    SCON = 0x50;    // Serial Mode 1 8 bit UART with timer1 baud rate
    TMOD = 0x20;    // timer 1 mode 2 8 bit auto reload | timer 0 mode 1 16 bit timer
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 1 means baud is determined by timer 1's overflow rate 
    REN = HIGH;        // serial port initialization
    EA = HIGH;			// enable interrupts
	ES = HIGH;			// enable serial port interrupts
    TR1 = HIGH; // start timer
    RI = LOW; // clear transmit interupt
}

void print_config(void) {
    serial_txstring("\n\rWizNet Adapter:\r\n\n");

    serial_txstring("Set MODE> . . . . MODE=");
    serial_txstring(server_state ? ((server_state == TCP) ? "TCP" : "BOTH") : "UDP");
    serial_txstring("\r\n");

    serial_txstring("Set RTU (0-9)>  . RTU=");
    serial_txstring(rtu);

    serial_txstring("\r\nSet IP> . . . . . IP=");
    serial_txstring(itoa(wiz_read(IP_1)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(IP_2)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(IP_3)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(IP_4)));

    serial_txstring("\r\nSet SUBNET> . . . SUB=");
    serial_txstring(itoa(wiz_read(SUBNET_1)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(SUBNET_2)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(SUBNET_3)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(SUBNET_4)));

    serial_txstring("\r\nSet GATE> . . . . GATE=");
     serial_txstring(itoa(wiz_read(GATEWAY_1)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(GATEWAY_2)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(GATEWAY_3)));
    serial_txchar('.');
    serial_txstring(itoa(wiz_read(GATEWAY_4)));

    serial_txstring("\r\nSet MAC>  . . . . MAC=");
    serial_txhex(wiz_read(MAC_1));
    serial_txhex(wiz_read(MAC_2));
    serial_txhex(wiz_read(MAC_3));
    serial_txhex(wiz_read(MAC_4));
    serial_txhex(wiz_read(MAC_5));
    serial_txhex(wiz_read(MAC_6));
    serial_txstring("\n\r\n");

}
// write to 4 address registers
void wiz_set_addr(uint16_t address, uint8_t len) {
    for (int i = 0; i < len; i ++) {
        wiz_write(address + i, addr[i]);
    }
}

// compares string against prepended command qualifier such as 'IP='
bool better_strncmp(char* check, uint8_t len) 
{
    for (uint8_t i = 0; i < len; i++) {
        if (serial_in[i] != check[i]) {
            return false;
        }
    }
    return true;
} 

// parses hex input for mac address
bool hex_parse(void) {
    if (serial_pt != 16) {
        return false;
    }
    // loop over buffer for max address length after MAC=
    char hex[3] = {'\0'};
    uint8_t num = 0;
    for (uint8_t i = 5; i < 16; i+=2) {
        // check for alphanumerical character
        if (isalnum(serial_in[i]) && isalnum(serial_in[i-1])) {
            // Cocatenate byte in ascii
            hex[0] = serial_in[i-1];
            hex[1] = serial_in[i];
            // convert byte hex string into n
            for (int i = 0; i < 2; i++) {
                num *= 16; // multiply by base 16 for every digit place
                // add hex value with ascii translation
                if (hex[i] >= '0' && hex[i] <= '9') {
                    num += hex[i] - '0';
                }
                else if (hex[i] >= 'A' && hex[i] <= 'F') {
                    num += hex[i] - 'A' + 10;
                }
                else {
                    return false;
                }
            }
            addr[(i-4)/2] = num;
        } 
        else {
            return false;
        }
    }
       
    return true;

}
// parse address syntax
bool addr_parse(uint8_t off) {
    uint8_t dots = 0;
    for (uint8_t i = 0; i < serial_pt; i++) {
        if (serial_in[i] == '.') {
            dots++;
        }
    }
    if (dots != 3) {
        return false;
    }
    if (serial_in[off] == '.') {
        return false;
    }
    char* ptr = strtok(serial_in+off, ".");
    uint16_t octet = 0;
    if (ptr == NULL) {
        return false;
    }

    dots = 0;
    do {
        octet = atoi(ptr);
        if (octet > 0 && octet <= 255) {
            addr[dots] = octet;
            dots++;
        }
        else if (octet == 0) {
            addr[dots] = 0;
            dots++;
        }
        else {
            return false;
        }

    }
    while (ptr = strtok(NULL, "."));

    return true;
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
    // wiz_set_gateway(126,10,220,254);   // 126.10.220.254 <---gateway address // 7e.a.dc.fe
    addr[0] = 126;
    addr[1] = 10;
    addr[2] = 200;
    addr[3] = 254;
    wiz_set_addr(GATEWAY_1, 4);

    // wiz_set_subnet(255,255,192,0);    // 255.255.192.0 <--- subnet mask // ff.ff.c0.0
    addr[0] = 255;
    addr[1] = 255;
    addr[2] = 192;
    addr[3] = 0;
    wiz_set_addr(SUBNET_1, 4);

    // wiz_set_mac(0x00, 0x08, 0xdc, 0x24, 0x4b, 0x5e); // mac address read as hex
    addr[0] = 0x00;
    addr[1] = 0x08;
    addr[2] = 0xdc;
    addr[3] = 0x24;
    addr[4] = 0x4b;
    addr[5] = 0x5e;
    wiz_set_addr(MAC_1, 6);
    
    // wiz_set_ip(126,10,210,10);
    addr[0] = 126;
    addr[1] = 10;
    addr[2] = 210;
    addr[3] = 10;
    wiz_set_addr(IP_1, 4);

    /* Init Sockets 0/1
       Socket 0: UDP 5000 | 4KB RX/TX | No Multicast
       Socket 1: TCP 5100 | 4KB RX/TX | Timeout ACK*/
    wiz_write(SOCKET0, 0x02); // set socket 0 to UDP with no multicast
    wiz_write(SOCKET1, 0x21); // set socket 1 to TCP without ack on internal timeout
    // wiz_write(SOCKET1, 0x01); // set socket 1 to TCP without ack on internal timeout
    /* Divide 8KB of RX memory and 8KB of TX memory over sockets 0 and 1
    Byte representation divided into 4 has 2 bits presenting each socket from greatest to least
    ex. 1100 0000 assigns all memory to socket 3 (11 for 8KB) */
    wiz_write(RX_MEM_SIZE, 0x0A);  // assign 4kb to s0/s1 each  0000 1010 
    wiz_write(TX_MEM_SIZE, 0x0A);   // assign 4kb to s0/s1 each 0000 1010
    /* Bind sockets to port numbers */
    wiz_write(SOCKET0_PORT_U, 0x13); // 5000 UDP Socket 0
    wiz_write(SOCKET0_PORT_L, 0x88);
     /* Bind sockets to port numbers */
    wiz_write(SOCKET1_PORT_U, 0x17); // 6000 TCP Socket 1
    wiz_write(SOCKET1_PORT_L, 0x70); 
}

void udp_open(void) 
{
    /* Initialize UDP Socket*/
    while (1) {
        wiz_write(SOCKET0_COM, OPEN); // open socket
        if (wiz_read(SOCKET0_STAT) != SOCK_UDP) {
            wiz_write(SOCKET0_COM, CLOSED); // socket not initialized, retry
        } else {
            break;
        }
    }
}

void udp_tx(uint16_t data_size, uint8_t *data)
{
    TX = LOW;
    uint16_t tx_free = 0x0000, txwr = 0x0000; 
    uint16_t tx_offset, tx_start_addr, upper_size, left_size;

    /* Get free TX Memory size */
    tx_free = tx_free | (wiz_read(SOCKET0_TXFSU) << 8);
    tx_free = tx_free | wiz_read(SOCKET0_TXFSL);
    


    /* Set destination ip,port */
    // wiz_write(SOCKET0_DIP1, ip1);
    // wiz_write(SOCKET0_DIP2, ip2);
    // wiz_write(SOCKET0_DIP3, ip3);
    // wiz_write(SOCKET0_DIP4, ip4);
    wiz_write(SOCKET0_DIP1, addr[0]);
    wiz_write(SOCKET0_DIP2, addr[1]);
    wiz_write(SOCKET0_DIP3, addr[2]);
    wiz_write(SOCKET0_DIP4, addr[3]);
    // wiz_write(SOCKET0_DPORU, (port >> 8) & 0xff);
    // wiz_write(SOCKET0_DPORL, port & 0xff);
    wiz_write(SOCKET0_DPORU, addr[4]);
    wiz_write(SOCKET0_DPORL, addr[5]);

    /* Calculate offset from write pointer*/
    txwr = txwr | (wiz_read(SOCKET0_TXWRU) << 8);
    txwr = txwr | wiz_read(SOCKET0_TXWRL);
    tx_offset = txwr & RXTX_MASK;
    
    /* Get start address*/
    tx_start_addr = SOCKET0_TX_BASE + tx_offset;
    // serial_txstring("\t start \0");
    // serial_txnum(tx_start_addr);
    // serial_ln();

    /* Overflow write to base address if overflow memory */
    if ( (tx_offset + data_size) > RXTX_MASK + 1) {
    serial_txstring("\t overflow \r\n\0");
        // copy upper_size bytes to start addr
        upper_size = (RXTX_MASK + 1) - tx_offset;
        wiz_write_buf(tx_start_addr, upper_size, data);
        left_size = (data_size - upper_size);
        wiz_write_buf(SOCKET0_TX_BASE, left_size, data + upper_size);
    }
    else {
        wiz_write_buf(tx_start_addr, data_size, data);
    }


    // update write pointer addr
    txwr += data_size;
    wiz_write(SOCKET0_TXWRU, (txwr >> 8) & 0xff);
    wiz_write(SOCKET0_TXWRL, txwr & 0xff);
    wiz_write(SOCKET0_COM, SEND);
    if (wiz_read(SOCKET0_COM == 0x00)) {
        serial_txstring("Send complete\r\n\0");
    } else if (wiz_read(SOCKET0_IR) & 0x08) {
        // check timeout bit
        serial_txstring("\r\nSend failed\r\n\0");
    }
    TX = HIGH;
}   


void udp_rx_helper(void) 
{
    // serial_txstring("------------------\r\n\0");
    uint16_t rx_offset, rx_start_addr, upper_size, left_size; // upper size stores uper size of start address, left stores left size of base addr
    uint8_t rxsizu, rxsizl; // stores upper and lower half of rx register size
    uint16_t peer_port = 0x0000, data_size = 0x0000, rxrd = 0x0000;
    uint16_t rx_size = 0x0000;
    // uint8_t peer_header[8] = {0};
    // uint8_t peer_ip[4] = {0};
    // uint8_t *peer_data = NULL;

    /* Get rx register size by combinning upper and lower half size values */
    rxsizu = wiz_read(SOCKET0_RXSIZU);
    rxsizl = wiz_read(SOCKET0_RXSIZL);
    rx_size = rx_size | rxsizl;
    rx_size = rx_size | (rxsizu << 8);

    /* Calculate offset */
    rxrd = rxrd | (wiz_read(SOCKET0_RXRDU) << 8);
    rxrd = rxrd | wiz_read(SOCKET0_RXRDL);
    rx_offset = rxrd & RXTX_MASK;

    /* Get start (physical) address*/
    rx_start_addr = SOCKET0_RX_BASE + rx_offset;
    // serial_txstring("\t start_addr: \0");
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
        serial_txstring("Overflow");
        upper_size = (RXTX_MASK + 1) - rx_offset; // get difference between end of buffer and offset
        // wiz_read_buf(rx_start_addr, upper_size, peer_header);
        wiz_read_buf(rx_start_addr, upper_size, addr); 

        left_size = UDP_HEADER_SIZE - upper_size; //
        // wiz_read_buf(SOCKET0_RX_BASE, left_size, peer_header + (upper_size - 1)); // read from overflow point base of rx 
        wiz_read_buf(SOCKET0_RX_BASE, left_size, addr + (upper_size - 1)); // read from overflow point base of rx
        // update offset past header
        rx_offset = left_size;
    }
    else {
        // copy header size bytes of start addresss to header addr (copy the header)
        // wiz_read_buf(rx_start_addr, UDP_HEADER_SIZE, peer_header);
        wiz_read_buf(rx_start_addr, UDP_HEADER_SIZE, addr);
        // update offset past header
        rx_offset += UDP_HEADER_SIZE;
    }
    // update start address 
    rx_start_addr = SOCKET0_RX_BASE + rx_offset;

    /* get remote peer information and receive data size from header*/
    // get port and size numbers by stitching upper and lower bytes
    // peer_port = peer_port | (peer_header[4] << 8);
    // peer_port = peer_port | peer_header[5];
    // data_size = data_size | (peer_header[6] << 8);
    // data_size = data_size | peer_header[7];
    peer_port = peer_port | (addr[4] << 8);
    peer_port = peer_port | addr[5];
    data_size = data_size | (addr[6] << 8);
    data_size = data_size | addr[7];


    // serial_txstring("Sender IP: ");
    // for (int i = 0; i < 4; i++) {
    //     // serial_txnum(peer_header[i]);
    //     serial_txstring(itoa(peer_header[i]));

    //     if (i < 3) {
    //         serial_txchar('.');
    //     }
    // }
    // serial_txstring("\tSender Port: ");
    // serial_txstring(itoa(peer_port));
    // serial_txstring("\tRX Size: ");
    // serial_txstring(itoa(rx_size));
    // serial_txchar('\n');

    // Allocate buffer for data size
    peer_data = malloc(sizeof(uint8_t) * data_size);

    /* Read Data
        1. Check if data size overflows rx buffer
            - Read data in two parts
    */
    if( (rx_offset + rx_size) > (RXTX_MASK + 1) ) {
        serial_txstring("Overflow");
        upper_size = (RXTX_MASK + 1) - rx_offset; // get first part of data
        wiz_read_buf(rx_start_addr, upper_size, peer_data);
        left_size = data_size - upper_size; // get remaining data 
        wiz_read_buf(rx_start_addr, left_size, peer_data + upper_size); // read from overflow point
    }
    else {
        wiz_read_buf(rx_start_addr, data_size, peer_data);
    }
    // serial_txstring("\rRESPONSE: ");
    // serial_txstring("Peer data: ");
    // serial_txchar(peer_data[0]);
    if (peer_data[0] == rtu[0]) {
        for (int i = 0; i < data_size; i++) { // Convert to uppercase
            if (peer_data[i] >= 'a' && peer_data[i] <= 'z' ) {
                peer_data[i] = peer_data[i] - 32;
                // serial_txchar(peer_data[i]);
            }
            else {
                // serial_txchar(peer_data[i]);
            }
        }
        // udp_tx(peer_header[0], peer_header[1], peer_header[2], peer_header[3], peer_port, data_size, peer_data);
        udp_tx(data_size, peer_data);

    }
    // else {
    //     // serial_txstring("Wrong RTU");
    //     udp_tx(peer_header[0], peer_header[1], peer_header[2], peer_header[3], peer_port, 29, "Incorrect format or wrong RTU\0");
    // }
    
    // serial_txchar('\n');
    // serial_ln();
    // serial_txstring("\r------------------\r\n\0");
    /* increase Sn_RX_RD as length of received packet*/
    rxrd += data_size + UDP_HEADER_SIZE;
    // store upper and lower halves
    wiz_write(SOCKET0_RXRDU, (rxrd >> 8) & 0xff);
    wiz_write(SOCKET0_RXRDL, rxrd&0xff);
    // serial_txstring("rxrd pointer end: \0");
    // Set received command
    wiz_write(SOCKET0_COM, RECV);
    // Clear s0_ir register by writing 1s
    wiz_write(SOCKET0_IR, 0x1f);
    free(peer_data);
}

void udp_rx(void) 
{
    if (wiz_read(SOCKET0_IR) & 0x04) { // check for Recv interrupt (bit 2/ 100 / x04)
    // serial_txstring("WHY DOES THIS HAPPENT O ME");
        RX = LOW;
        udp_rx_helper();
        RX = HIGH;
    }
}
/* Initialize TCP Socket in Server Mode*/
void tcp_open(void) 
{
    while (1) {
        wiz_write(SOCKET1_COM, OPEN); // open socket
        if (wiz_read(SOCKET1_STAT) != SOCK_INIT) {
            serial_txstring("z");
            wiz_write(SOCKET1_COM, CLOSED); // socket not initialized, retry
            // serial_txstring("infinite\r\n\0");
            continue;
        } 
        wiz_write(SOCKET1_COM, LISTEN); // SET SOCKET TO SERVER MODE LISTEN
        if (wiz_read(SOCKET1_STAT) != SOCK_LISTEN) {
            wiz_write (SOCKET1_COM, CLOSED);
            // serial_txstring("z");
            continue;
        }
        else {
            break;
        }
    }
}
void tcp_close_state(void) {
    // check for FIN|| (wiz_read(SOCKET1_IR) & 0x08)
    if ((wiz_read(SOCKET1_IR) & 0x02) || (wiz_read(SOCKET1_IR) & 0x08) ) { // x02 = discon  x08 = timeout  close on interrupt
        // serial_txstring("closed??\t"); 
        wiz_write(SOCKET1_COM, CLOSED);
        wiz_write(SOCKET1_IR, 0x1f);
        tcp_open();
    }


}
void tcp_tx(uint16_t data_size) {
    TX = LOW;
    uint16_t tx_free = 0x0000, txwr = 0x0000; 
    uint16_t tx_offset, tx_start_addr, upper_size, left_size;

    /* Get free TX Memory size */
    tx_free = tx_free | (wiz_read(SOCKET1_TXFSU) << 8);
    tx_free = tx_free | wiz_read(SOCKET1_TXFSL);

    /* Calculate offset from write pointer*/
    txwr = txwr | (wiz_read(SOCKET1_TXWRU) << 8);
    txwr = txwr | wiz_read(SOCKET1_TXWRL);
    tx_offset = txwr & RXTX_MASK;
    
    /* Get start address*/
    tx_start_addr = SOCKET1_TX_BASE + tx_offset;
    // serial_txstring("\t start \0");
    // serial_txnum(tx_start_addr);
    // serial_ln();

    /* Overflow write to base address if overflow memory */
    if ( (tx_offset + data_size) > RXTX_MASK + 1) {
    serial_txstring("\t overflow \r\n\0");
        // copy upper_size bytes to start addr
        upper_size = (RXTX_MASK + 1) - tx_offset;
        wiz_write_buf(tx_start_addr, upper_size, peer_data);
        left_size = (data_size - upper_size);
        wiz_write_buf(SOCKET0_TX_BASE, left_size, peer_data + upper_size);
    }
    else {
        wiz_write_buf(tx_start_addr, data_size, peer_data);
    }


    // update write pointer addr
    txwr += data_size;
    wiz_write(SOCKET1_TXWRU, (txwr >> 8) & 0xff);
    wiz_write(SOCKET1_TXWRL, txwr & 0xff);
    wiz_write(SOCKET1_COM, SEND);

    if (wiz_read(SOCKET1_COM == 0x00)) {
        serial_txstring("Send complete\r\n\0");
    } else if (wiz_read(SOCKET0_IR) & 0x08) {
        // check timeout bit
        serial_txstring("\r\nSend failed\r\n\0");
    }
    TX = HIGH;
}
void tcp_rx_helper(void) {
    uint16_t rx_offset, rx_start_addr, upper_size, left_size; // upper size stores uper size of start address, left stores left size of base addr
    uint8_t rxsizu, rxsizl; // stores upper and lower half of rx register size
    uint16_t rxrd = 0x0000;
    uint16_t rx_size = 0x0000;
    // uint8_t *peer_data = NULL;

    /* Get rx register size by combinning upper and lower half size values */
    rxsizu = wiz_read(SOCKET1_RXSIZU);
    rxsizl = wiz_read(SOCKET1_RXSIZL);
    rx_size = rx_size | rxsizl;
    rx_size = rx_size | (rxsizu << 8);

    /* Calculate offset */
    rxrd = rxrd | (wiz_read(SOCKET1_RXRDU) << 8);
    rxrd = rxrd | wiz_read(SOCKET1_RXRDL);
    rx_offset = rxrd & RXTX_MASK;

    /* Get start (physical) address*/
    rx_start_addr = SOCKET1_RX_BASE + rx_offset;
    // serial_txstring("\t start_addr: \0");
    // serial_txnum(rx_start_addr);

    peer_data = malloc(sizeof(uint8_t) * rx_size);
    // Handle overflow socket memory
    if ( (rx_offset + rx_size) > (RXTX_MASK + 1) ) {
        serial_txstring("Overflow");
        upper_size = (RXTX_MASK + 1) - rx_offset; // get difference between end of buffer and offset
        wiz_read_buf(rx_start_addr, upper_size, peer_data); 
        left_size = rx_size - upper_size; // get the remaining amount of data 
        wiz_read_buf(SOCKET1_RX_BASE, left_size, peer_data + (upper_size - 1)); // read from overflow point base of rx
    }
    else {
        // copy header size bytes of start addresss to header addr (copy the header)
        wiz_read_buf(rx_start_addr, rx_size, peer_data);
    }

    /* increase Sn_RX_RD as length of received packet*/
    rxrd += rx_size;
    // store upper and lower halves
    wiz_write(SOCKET1_RXRDU, (rxrd >> 8) & 0xff);
    wiz_write(SOCKET1_RXRDL, rxrd&0xff);

    wiz_write(SOCKET1_COM, RECV);
    // Clear s1_ir register by writing 1s
    wiz_write(SOCKET1_IR, 0x1f);

    if (peer_data[0] == rtu[0]) {
        for (int i = 0; i < rx_size; i++) { // Convert to uppercase
            if (peer_data[i] >= 'a' && peer_data[i] <= 'z' ) {
                peer_data[i] = peer_data[i] - 32;
                // serial_txchar(peer_data[i]);
            }
            // else {
            //     // serial_txchar(peer_data[i]);
            // }
        }
        tcp_tx(rx_size);
    }
    // else {
    //     free(peer_data);
    //     peer_data = malloc(sizeof(uint8_t) * 30);
    //     peer_data = "Incorrect format or wrong RTU\0";
    //     tcp_tx(30);

    // }
    free(peer_data);
}
void tcp_rx(void) {
    if (wiz_read(SOCKET1_IR) & 0x04) {  // check for Recv interrupt (bit 2/100/x04)
        RX = LOW;
        tcp_rx_helper();
        RX = HIGH;
    }
}



void main(void)
{
    delay_us(1000);
    setup();
    CLK = LOW; // set clock idle state
    wiz_init();
    udp_open(); // open udp socket
    uint8_t read;
	while (1) {
        if (server_state == UDP) {
            udp_rx();
        }
        else if (server_state == TCP) {
            tcp_rx();
            tcp_close_state();
        }
        else {
            udp_rx();
            tcp_rx();
            tcp_close_state();
        }
        read = RX_data(); // get character from terminal
 
        /* Take input from valid keys and store in buffer*/
        if (((read >= '0' && read <= '9') 
            || (read >= 'a' && read <= 'z') 
                || (read >= 'A' && read <= 'Z') 
                    || (read == SPACE) || (read == '.') || (read == '?') || (read == '=') )
            && serial_pt < 21) {
            serial_in[serial_pt] = read;
            serial_pt++;
            serial_txchar(read);
        }
        /* Handle backspace on terminal */
        if (read == BACK && serial_pt > 0) {
            serial_in[serial_pt] = '\0';
            serial_pt--;
            serial_txchar(BACK); // set cursor back one
            serial_txchar(SPACE); // insert 'empty' character
            serial_txchar(BACK); // set cursor back
        }

        if (read == ENTER && serial_pt > 0) 
        {
            // serial_txnum(read);
            serial_in[serial_pt] = '\0';
            serial_txstring("\r\nCOM>");
            serial_txstring(serial_in);
            serial_txstring("\r\n\r\n");
            // evaluate config menu
            if (serial_pt == 1 && serial_in[0] == '?') {
                print_config();
            } else {
                if (better_strncmp("IP=", 3) && addr_parse(3) ) {
                    wiz_set_addr(IP_1, 4);
                    // wiz_write(MODE, 0X80); // perform s/w reset
                } 
                else if(better_strncmp("RTU=", 4) && serial_pt == 5) {
                    if (serial_in[4] >= '0' && serial_in[4] <= '9') {
                        rtu[0] = serial_in[4];
                    }
                }
                else if(better_strncmp("SUB=", 4) && addr_parse(4)) {
                    wiz_set_addr(SUBNET_1, 4);
                    // wiz_write(MODE, 0X80); // perform s/w reset
                }
                else if(better_strncmp("MAC=", 4) && hex_parse()) {
                    wiz_set_addr(MAC_1, 6);
                }
                else if(better_strncmp("GATE=", 5) && addr_parse(5)) {
                    wiz_set_addr(GATEWAY_1, 4);
                    // wiz_write(MODE, 0X80); // perform s/w reset

                }
                else if (better_strncmp("MODE=", 5) && (serial_pt == 8 && 
                ((serial_in[5] == 'T' && serial_in[6] == 'C' && serial_in[7] == 'P') ||
                (serial_in[5] == 'U' && serial_in[6] == 'D' && serial_in[7] == 'P'))) ||
                (serial_pt == 9 && (serial_in[5] == 'B' && serial_in[6] == 'O' && serial_in[7] == 'T' && serial_in[8] == 'H'))) 
                {
                    // close active sockets and open corresponding sockets
                    if (serial_in[5] == 'B') {
                        // set both >> TODO: CHECK IF OPENING AN ALREADY OPEN SOCKET CAUSES PRBLEMS
                        server_state = BOTH;
                        wiz_write(SOCKET0_COM, CLOSED); // close udp
                        wiz_write(SOCKET1_COM, CLOSED);  // close tcp

                        udp_open();
                        tcp_open();
                        serial_txstring("UDP/TCP:5000/6000 OPEN\r\n\0");

                    } 
                    else if (serial_in[5] == 'T' && server_state != TCP) {
                        server_state = TCP;
                        wiz_write(SOCKET0_COM, CLOSED); // close udp
                        tcp_open();
                        serial_txstring("TCP:6000 OPEN\r\n\0");
                    }
                    else if (serial_in[5] == 'U'&& server_state != UDP) {
                        server_state = UDP;
                        wiz_write(SOCKET1_COM, CLOSED); // close tcp
                        udp_open();
                        serial_txstring("UDP:5000 OPEN\r\n\0");
                    }
                } 
                else {
                    serial_txstring("Invalid, try <?>\r\n\r\n");
                }
            }

            // clear buffer
            for (int i = 0; i < 22; i++) {
                serial_in[i] = '\0';
            }
            serial_pt = 0;
        }
    }

}
