#ifndef CONSTANT_H
#define CONSTANT_H
#define MSK_8 0x80
#define MSK_16 0x8000

#define OP_WRITE 0xf0
#define OP_READ 0x0f

// S0/S1 Registers
#define SOCKET_0 0x0400
#define SOCKET_0_PORT_U 0x0404
#define SOCKET_0_PORT_L 0x0405
#define SOCKET_1 0x0500
#define SOCKET_1_PORT_U 0x0504
#define SOCKET_1_PORT_L 0x0505

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



#define HIGH 1
#define LOW 0
#define BYTE_8 8
#define BYTE_16 16
#endif
