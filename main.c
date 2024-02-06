#include <8051.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
int buf_counter = 0;
// #include "spi.h"
#include "periph.h"
#include "constant.h"

// Initial configuartion setting and code sourced from: https://exploreembedded.com/wiki/8051_Family_C_Library

/*-------------------------------------------------------------------------------
                         void spi_init()
----------------------------------------------------------------------------------
 * Arguments: none
 * Return: none
 
 * description: This function is used to initialize the spi in shift register mode with a baud rate of SYSclk/12

    1.  8-Bit Shift Register MODE0. Data bits are transmitted/received LSB
                    RxD is used for data i/o, TxD is used for clock
                    Baud is fixed at 1/12 System clock cycle
    2.  9600 baud rate configured by setting TH1
    3.      SCON is configured in MODE0 ie. 8bit Data 1-Start and 1-Stop bit
    4.      Finally the timer is turned ON by setting TR1 bit to generate the baud rate
----------------------------------------------------------------------------------*/
void spi_init(void)
{
    // SCON = SCON & 0x0F; 
    SCON = 0x50;    // serial mode 3 8 bit data 1 start and 1 stop bit
    TMOD = 0x20;    // timer 1 mode 2 and timer 2 mode 0 ---> mode 2 is 16bit timer/counter auto reload and thx/tlx are cascaded
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 3 means baud is determined by timer 1's overflow rate 
    REN = 1;        // serial port initialization
    EA = 1;			// enable interrupts
	ES = 1;			// enable serial port interrupts
    TR1 = 1;        // Turn ON timer 1

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

void spi_txchar(char ch)
{
    SBUF=ch;       // Load the data to be transmitted
    while(TI==0);    // Wait till the data is trasmitted
        TI=0;         //Clear the flag for next cycle.
}

void spi_txstring(char *string_ptr)
 {
          while(*string_ptr)
           spi_txchar(*string_ptr++);
}

unsigned char RX_data(void)
{
    unsigned char a;
    while(RI == 0);
    RI = 0;
    a = SBUF;
    spi_txchar(a);
    return a;
}


// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) {
    uint8_t command = OP_WRITE;
    // send op code
    for (uint8_t i = 0; i < 8; i++) {
        if (command & MSK_8) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        CLK = 1;
        command = command << 1;
        CLK = 0;
    }

    // send address field
    for (uint8_t i = 0; i < 16; i++) {
        if (addr & MSK_16) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        CLK = 1;
        addr = addr << 1;
        CLK = 0;
    }

    // send data
    for (uint8_t i = 0; i < 8; i++) {
        if (data & MSK_8) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        data = 1;
        command = command << 1;
        data = 0;
    }

}

// OP Read Instruction
void wiz_read(uint16_t addr, uint8_t data) {
    uint8_t command = OP_READ;
    // send op code
    for (uint8_t i = 0; i < 8; i++) {
        if (command & MSK_8) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        CLK = 1;
        command = command << 1;
        CLK = 0;
    }

    // send address field
    for (uint8_t i = 0; i < 16; i++) {
        if (addr & MSK_16) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        CLK = 1;
        addr = addr << 1;
        CLK = 0;
    }

    // send data
    for (uint8_t i = 0; i < 8; i++) {
        if (data & MSK_8) {
            MOSI = 1;
        }
        else {
            MOSI = 0;
        }
        data = 1;
        command = command << 1;
        data = 0;
    }

}

// sends byte instructions
void cmdout(unsigned char cmd) 
{
    for (unsigned char i = 0; i < 8; i++) {
        if (cmd & MSK) {
            SDA = 1;
        }
        else {
            SDA = 0;
        }
        delay(10);
        SCL = 1;
        cmd = cmd << 1;  // clock data into sda
        SCL = 0;
    }
}


void main(void)
{

    // spi_transmit();
    spi_init();
	while (1) {
        RX_data();
    }
}