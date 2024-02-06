#include <8051.h>
// #include "spi.h"
// #include "periph.h"
// #include "constant.h"

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
    SCON = 0xD0;    // serial mode 1 8 bit data 1 start and 1 stop bit
    TMOD = 0x20;    // timer 1 mode 2 and timer 2 mode 0 ---> mode 2 is 16bit timer/counter and thx/tlx are cascaded
    TH1 = 0xfd;     // Load value for 9600 baud --> UART Mode 3 means baud is determined by timer 1's overflow rate 
    // TL1 = 0xfd;
    // PCON = 0x00; // SMODE = 0 for 9600 baud

    // REN = 1;        // serial port initialization
    // TMOD |= 0x01;   // mode 1, 16-bit timer - init timer 0 
    // TH0 = 0x00;		// overflow at 65536
	// TL0 = 0x00;
    // EA = 1;			// enable global interrupts
	// ES = 1;			// enable serial port interrupt
    // ET1 = 1; // timer 1 interrupts
    // TR0 = 1;		// timer on
    TR1 = 1;        // Turn ON timer 1

}

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

void main(void)
{
    // spi_init();
    // spi_transmit();
	while (1) {
        spi_init();
        spi_txstring("Hello World\n");
    }
}