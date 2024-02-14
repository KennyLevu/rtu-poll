#include <8051.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "periph.h"
#include "constant.h"
#include "serial.h"
#include "wiz.h"

uint8_t rtu = '0';
uint8_t serial_in[22] = {'\0'};
uint8_t serial_pt = 0;
static const char com_rtu[5] = "RTU=";
static const char com_ip[4] = "IP=";
static const char com_sub[5] = "SUB=";
static const char com_gate[6] = "GATE=";
static const char com_mac[5] = "MAC=";

bool better_strncmp(char* check, uint8_t len) 
{
    for (uint8_t i = 0; i < len; i++) {
        if (serial_in[i] != check[i]) {
            return false;
        }
    }
    return true;
} 
// uint8_t com_check(void) 
// {
//     if (better_strncmp("IP=",3) == true) {
//     //     serial_txstring("change ip\r\n");
//         return 0;
//     }
//     return 0;
// }

void serial_txreg(uint16_t addr)
{
    serial_txnum(wiz_read(addr));
    serial_ln();
}

void serial_tx2reg(uint16_t upper, uint16_t lower)
{
    uint16_t combined = 0x0000; 
    uint8_t up = wiz_read(upper);
    uint8_t lo = wiz_read(lower);
    combined = combined | lo;
    combined = combined | (up << 8);
    serial_txnum(combined);
    serial_ln();
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
    wiz_write(GATEWAY_1, 126);
    wiz_write(GATEWAY_2, 10);
    wiz_write(GATEWAY_3, 220);
    wiz_write(GATEWAY_4, 254);

    // wiz_set_subnet(255,255,192,0);    // 255.255.192.0 <--- subnet mask // ff.ff.c0.0
    wiz_write(SUBNET_1, 255);
    wiz_write(SUBNET_2, 255);
    wiz_write(SUBNET_3, 192);
    wiz_write(SUBNET_4, 0);

    // wiz_set_mac(0x00, 0x08, 0xdc, 0x24, 0x4b, 0x5e); // mac address read as hex
    wiz_write(MAC_1, 0x00);
    wiz_write(MAC_2, 0x08);
    wiz_write(MAC_3, 0xdc);
    wiz_write(MAC_4, 0x24);
    wiz_write(MAC_5, 0x4b);
    wiz_write(MAC_6, 0x5e);
    
    // wiz_set_ip(126,10,210,10);     // 126.10.218.163 <--- my pc // 126.10.200.0 7e.a.c8.0<--- set mcu
    wiz_write(IP_1, 126);
    wiz_write(IP_2, 10);
    wiz_write(IP_3, 210);
    wiz_write(IP_4, 10);




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
    wiz_write(SOCKET0_PORT_U, 0x13);
    wiz_write(SOCKET0_PORT_L, 0x88);
    // wiz_set_port(0,0x13,0x88); // set socket 0 udp port to 5000 0x1388
    // wiz_set_port(1,0x13,0xec);   // set socket 1 tcp port to 5100 0x13ec

    
    /* Initialize TCP Socket*/
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
    /* Initialize UDP Socket*/
    while (1) {
        wiz_write(SOCKET0_COM, OPEN); // open socket
        if (wiz_read(SOCKET0_STAT) != SOCK_UDP) {
            wiz_write(SOCKET0_COM, CLOSED); // socket not initialized, retry
        } else {
            serial_txstring("UDP Socket 0 port 5000 open\r\n\0");
            break;
        }
    }
}

void udp_tx(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint16_t port, uint16_t data_size, uint8_t *data)
{

    uint16_t tx_free = 0x0000, txwr = 0x0000; 
    uint16_t tx_offset, tx_start_addr, upper_size, left_size;

    /* Get free TX Memory size */
    tx_free = tx_free | (wiz_read(SOCKET0_TXFSU) << 8);
    tx_free = tx_free | wiz_read(SOCKET0_TXFSL);
    


    /* Set destination ip,port */
    wiz_write(SOCKET0_DIP1, ip1);
    wiz_write(SOCKET0_DIP2, ip2);
    wiz_write(SOCKET0_DIP3, ip3);
    wiz_write(SOCKET0_DIP4, ip4);
    wiz_write(SOCKET0_DPORU, (port >> 8) & 0xff);
    wiz_write(SOCKET0_DPORL, port & 0xff);

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
    // while(1) {
    wiz_write(SOCKET0_COM, SEND);
    if (wiz_read(SOCKET0_COM == 0x00)) {
        serial_txstring("Send complete\r\n\0");
    } else if (wiz_read(SOCKET0_IR) & 0x08) {
        // check timeout bit
        serial_txstring("\r\nSend failed\r\n\0");
    }
// }
}   


void udp_rx_helper(void) 
{
    // serial_txstring("------------------\r\n\0");
    uint16_t rx_offset, rx_start_addr, upper_size, left_size; // upper size stores uper size of start address, left stores left size of base addr
    uint8_t rxsizu, rxsizl; // stores upper and lower half of rx register size
    uint16_t peer_port = 0x0000, data_size = 0x0000, rxrd = 0x0000;
    uint16_t rx_size = 0x0000;
    uint8_t peer_header[8] = {0};
    // uint8_t peer_ip[4] = {0};
    uint8_t *peer_data = NULL;

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
        wiz_read_buf(rx_start_addr, upper_size, peer_header); 
        left_size = UDP_HEADER_SIZE - upper_size; // get the remaining amount of data 
        wiz_read_buf(SOCKET0_RX_BASE, left_size, peer_header + (upper_size - 1)); // read from overflow point base of rx
        // update offset past header
        rx_offset = left_size;
    }
    else {
        // copy header size bytes of start addresss to header addr (copy the header)
        wiz_read_buf(rx_start_addr, UDP_HEADER_SIZE, peer_header);
        // update offset past header
        rx_offset += UDP_HEADER_SIZE;
    }
    // update start address 
    rx_start_addr = SOCKET0_RX_BASE + rx_offset;

    /* get remote peer information and receive data size from header*/
    // get port and size numbers by stitching upper and lower bytes
    peer_port = peer_port | (peer_header[4] << 8);
    peer_port = peer_port | peer_header[5];
    data_size = data_size | (peer_header[6] << 8);
    data_size = data_size | peer_header[7];

    serial_txstring("Sender IP: ");
    for (int i = 0; i < 4; i++) {
        serial_txnum(peer_header[i]);
        if (i < 3) {
            serial_txchar('.');
        }
    }
    serial_txstring("\tSender Port: ");
    serial_txnum(peer_port);
    serial_txstring("\tRX Size: ");
    serial_txnum(rx_size);
    serial_ln();

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
    serial_txstring("\rRESPONSE: ");
    if (peer_data[0] == rtu) {
        for (int i = 0; i < data_size; i++) { // Convert to uppercase
            if (peer_data[i] >= 'a' && peer_data[i] <= 'z' ) {
                peer_data[i] = peer_data[i] - 32;
                serial_txchar(peer_data[i]);
            }
            else {
                serial_txchar(peer_data[i]);
            }
        }
        udp_tx(peer_header[0], peer_header[1], peer_header[2], peer_header[3], peer_port, data_size, peer_data);
    }
    else {
        serial_txstring("Wrong RTU");
        udp_tx(peer_header[0], peer_header[1], peer_header[2], peer_header[3], peer_port, 29, "Incorrect format or wrong RTU\0");
    }
    
    serial_ln();
    serial_txstring("\r------------------\r\n\0");
    /* increase Sn_RX_RD as length of received packet*/
    rxrd += data_size + UDP_HEADER_SIZE;
    // store upper and lower halves
    wiz_write(SOCKET0_RXRDU, (rxrd >> 8) & 0xff);
    wiz_write(SOCKET0_RXRDL, rxrd&0xff);
    // serial_txstring("rxrd pointer end: \0");
    // Set received command
    wiz_write(SOCKET0_COM, RECV);
    // Clear s0_ir register by writing 1s
    wiz_write(SOCKET0_IR, 0xff);
    free(peer_data);
}

void udp_rx(void) 
{
    if (wiz_read(SOCKET0_IR) & 0x04) { // check for Recv interrupt (bit 2/ 100 / x04)
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
    TR1 = HIGH; // start timer
    RI = LOW; // clear transmit interupt
}

void print_config(void) {
    serial_txstring("\n\rWizNet Adapter:\r\n\n");

    serial_txstring("   IPv4 Address  . . : ");
    serial_txnum(wiz_read(IP_1));
    serial_txchar('.');
    serial_txnum(wiz_read(IP_2));
    serial_txchar('.');
    serial_txnum(wiz_read(IP_3));
    serial_txchar('.');
    serial_txnum(wiz_read(IP_4));
    serial_txstring("\r\n");

    serial_txstring("   RTU Address . . . : ");
    serial_txnum(rtu);
    serial_txstring("\r\n");

    serial_txstring("   Subnet Mask . . . : ");
    serial_txnum(wiz_read(SUBNET_1));
    serial_txchar('.');
    serial_txnum(wiz_read(SUBNET_2));
    serial_txchar('.');
    serial_txnum(wiz_read(SUBNET_3));
    serial_txchar('.');
    serial_txnum(wiz_read(SUBNET_4));
    serial_txstring("\r\n");

    serial_txstring("   Gateway Address . : ");
    serial_txnum(wiz_read(GATEWAY_1));
    serial_txchar('.');
    serial_txnum(wiz_read(GATEWAY_2));
    serial_txchar('.');
    serial_txnum(wiz_read(GATEWAY_3));
    serial_txchar('.');
    serial_txnum(wiz_read(GATEWAY_4));
    serial_txstring("\r\n");

    serial_txstring("   MAC Address . . . : ");
    serial_txhex(wiz_read(MAC_1));
    serial_txchar(':');
    serial_txhex(wiz_read(MAC_2));
    serial_txchar(':');
    serial_txhex(wiz_read(MAC_3));
    serial_txchar(':');
    serial_txhex(wiz_read(MAC_4));
    serial_txchar(':');
    serial_txhex(wiz_read(MAC_5));
    serial_txchar(':');
    serial_txhex(wiz_read(MAC_6));
    serial_txstring("\n\r\n");

    serial_txstring("Set RTU (0-9): USING RTU=\r\n");
    serial_txstring("Set IP: USING IP=\r\n");
    serial_txstring("Set SUBNET: USING SUB=\r\n");
    serial_txstring("Set GATE: USING GATE=\r\n");
    serial_txstring("Set MAC: USING MAC=\r\n\n");

}
void main(void)
{
    delay_us(1000);
    setup();
    CLK = LOW; // set clock idle state
    wiz_init();
    udp_open(); // open udp socket
    uint8_t read;
    // serial_txstring("Program started");
	while (1) {
    // serial_txstring("Program started");
        udp_rx();
        read = RX_data(); // get character from terminal
        // if (read != 0) {
        //     serial_txnum(read);
        // }

        /* Take input from valid keys and store in buffer*/
        if (((read >= '0' && read <= '9') 
            || (read >= 'a' && read <= 'z') 
                || (read >= 'A' && read <= 'Z') 
                    || (read == SPACE) || (read == '.') || (read == ':') || (read == '?') || (read == '=') )
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

        if (read == ENTER && serial_pt > 0) {
            // serial_txnum(read);
            serial_in[serial_pt] = '\0';
            serial_txstring("\r\nCOM>");
            serial_txstring(serial_in);
            serial_txstring("\r\n");
            // // evaluate config menu
            if (serial_pt == 1 && serial_in[0] == '?') {
                print_config();
            } else {
                if (better_strncmp("IP=", 3)) {
                    serial_txchar('2\n');
                } 
                else if(better_strncmp("RTU=", 4)) {
                    serial_txchar('1');
                }
                else if(better_strncmp("SUB=", 4)) {
                    serial_txchar('3');
                }
                else if(better_strncmp("MAC=", 4)) {
                    serial_txchar('5');
                }
                else if(better_strncmp("GATE=", 5)) {
                    serial_txchar('4');
                }
                else {
                    serial_txstring("Invalid, try <?>\r\n");
                }
            }

            // clear buffer
            for (int i = 0; i < 22; i++) {
                serial_in[i] = '\0';
            }
            serial_pt = 0;
            // for (; serial_pt > 0; serial_pt--) {
            //     serial_in[serial_pt] = '\0';
            // }
        }
    }

}
