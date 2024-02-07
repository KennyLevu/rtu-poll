#ifndef CONSTANT_H
#define CONSTANT_H
#define MSK_8 0x80
#define MSK_16 0x8000

#define OP_WRITE 0xf0
#define OP_READ 0x0f

// S0/S1 Registers Scocket n Mode Register
#define SOCKET0 0x0400 // socket mode
#define SOCKET0_COM 0x0401 // command register
#define SOCKET0_STAT 0x403 // status register
#define SOCKET0_PORT_U 0x0404 // upper half port number
#define SOCKET0_PORT_L 0x0405 // lower half port number

#define SOCKET1 0x0500
#define SOCKET1_COM 0x0501
#define SOCKET1_STAT 0x503
#define SOCKET1_PORT_U 0x0504
#define SOCKET1_PORT_L 0x0505

// GAR Register
#define GATEWAY_1 0x0001
#define GATEWAY_2 0x0002
#define GATEWAY_3 0x0003
#define GATEWAY_4 0x0004

// SUBR register
#define SUBNET_1 0x0005
#define SUBNET_2 0x0006
#define SUBNET_3 0x0007
#define SUBNET_4 0x0008

// SHAR register
#define MAC_1 0x0009
#define MAC_2 0x000a
#define MAC_3 0x000b
#define MAC_4 0x000c
#define MAC_5 0x000d
#define MAC_6 0x000e

// SIPR Register
#define IP_1 0x000F
#define IP_2 0x0010
#define IP_3 0x0011
#define IP_4 0x0012

// Mode Register
#define MODE 0x0000

// Interrupt Mask Rgister
#define IMR 0x0016

// Retry Time Value Register
#define RETRY_U 0x0017
#define RETRY_L 0x0018

// Retry Count Register
#define RETRY_COUNT 0x0019

// RX Memory Size RMSR
#define RX_MEM_SIZE 0x001a 
// TX Memory Size TMSR
#define TX_MEM_SIZE 0x001b


/* SOCKET STATUS COMMANDS */
#define SOCK_CLOSED 0X00
#define SOCK_INIT 0x13
#define SOCK_LISTEN 0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_UDP 0x22
#define OPEN 0x01
#define LISTEN 0x02
#define CONNECT 0x04
#define DISCON 0x08
#define SEND 0x20
#define SEND_MAC 0x21
#define SEND_KEEP 0x22
#define RECV 0x40
#define CLOSED 0x10

#define HIGH 1
#define LOW 0
#define BYTE_8 8
#define BYTE_16 16
#endif
