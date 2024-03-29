#ifndef CONSTANT_H
#define CONSTANT_H
#define MSK_8 0x80
#define MSK_16 0x8000

#define OP_WRITE 0xf0
#define OP_READ 0x0f
#define ENTER 13 // Enter key ascii tera term
#define ESC 27 // Escape key ascii tera term
#define BACK 8 // Backspace key ascii tera term
#define SPACE 32 // space
#define UDP 0
#define TCP 1
#define BOTH 2
// IR Interrupt Register (read only)
#define INTER_REG 0x0015
#define SOCK0_MSK 0x01 // interrupt mask
// S0/S1 Registers Scocket n Mode Register
#define SOCKET0 0x0400 // socket mode
#define SOCKET0_COM 0x0401 // command register
#define SOCKET0_IR 0x0402 // Socket n interrupt register
#define SOCKET0_STAT 0x0403 // status register
#define SOCKET0_PORT_U 0x0404 // upper half port number
#define SOCKET0_PORT_L 0x0405 // lower half port number
#define SOCKET0_TXFSU 0x0420 // upper half of tx free size register
#define SOCKET0_TXFSL 0x0421 // lower half of tx free size register
#define SOCKET0_TXWRU 0x0424 // upper half of tx write pointer register
#define SOCKET0_TXWRL 0x0425 // lower half of tx write pointer register
#define SOCKET0_RXSIZU 0x0426 // upper half of size of rx register
#define SOCKET0_RXSIZL 0x0427 // lower half of size of rx register
#define SOCKET0_RXRDU 0x0428 // upper half of read pointer register, shows locations of data
#define SOCKET0_RXRDL 0x0429 // loewr half of rdrx
#define SOCKET0_DIP1 0x040c // destination ip 
#define SOCKET0_DIP2 0x040d
#define SOCKET0_DIP3 0x040e
#define SOCKET0_DIP4 0x040f
#define SOCKET0_DPORU 0x0410 // dest socket port upper half
#define SOCKET0_DPORL 0x0411

// TXRX Memory Base ADDR from datasheet
#define SOCKET0_RX_BASE 0x6000 
#define SOCKET0_TX_BASE 0x4000

/* RX Mask = Memory size (4KB - 1) == 0xFFFF - 1
 mask helps calculate offset by wrapping the offset value within the register size */
#define RXTX_MASK 0x1000 - 1 

// Socket 1 base addresses start on top of socket0 base address + memory size = (mask==(4kb -1) +1)
#define SOCKET1_RX_BASE (SOCKET0_RX_BASE + RXTX_MASK + 1)
#define SOCKET1_TX_BASE (SOCKET0_TX_BASE + RXTX_MASK + 1)

#define SOCKET1 0x0500
#define SOCKET1_COM 0x0501
#define SOCKET1_IR 0x0502
#define SOCKET1_STAT 0x503
#define SOCKET1_PORT_U 0x0504
#define SOCKET1_PORT_L 0x0505
#define SOCKET1_TXFSU 0x0520
#define SOCKET1_TXFSL 0x0521
#define SOCKET1_TXWRU 0x0524
#define SOCKET1_TXWRL 0x0525
#define SOCKET1_RXSIZU 0x0526
#define SOCKET1_RXSIZL 0x0527
#define SOCKET1_RXRDU 0x0528
#define SOCKET1_RXRDL 0x0529
#define SOCKET1_DIP1 0x050c
#define SOCKET1_DIP2 0x050d
#define SOCKET1_DIP3 0x050e
#define SOCKET1_DIP4 0x050f
#define SOCKET1_DPORU 0x0510
#define SOCKET1_DPORL 0x0511

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

// UDP Header Size 8 bytes
#define UDP_HEADER_SIZE 8

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
