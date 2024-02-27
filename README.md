Resource: https://wiki.laptop.org/go/SDCC#Compiling_a_program
w5100 datashet: https://www.sparkfun.com/datasheets/DevTools/Arduino/W5100_Datasheet_v1_1_6.pdf


This project emulates a remote telemetery unit acting as a server that responds to poll requests from master controllers. The project was built with an STC89 microcontroller interfacing with a Wiznet5100 over a serial port interface. Running the curse.py script will launch a curses interface which emulates an MCU polling for the periphal Wiznet board (the rtu).

The C program should be flashed on an stc89 compatible board with the appropriate pinouts specified in periph.h - The program supports a user interface for network configuration over serial communication. Otherwise, a default config can be initialized in main.c, the serial menu supports toggling between UDP and TCP protocols.

Steps:
1. Compile C files using sdcc
2. Flash program using stcgal
OR
1. Compile and flash by running make

The ﬁrst two ﬁles will need to be compiled separately with the commands:
sdcc -c --model-small--stack-auto wiz.c

sdcc -c --model-small --stack-auto serial.c

Then compile the source ﬁle containing the main() function and link the ﬁles together with the following command:

sdcc --model-small --stack-auto main.c serial.rel wiz.rel 

ISP Flash the ihx file using stcgal:
stcgal -p COM8 -P stc89 -o cpu_6t_enabled=true  main.ihx

Notes:
-p sets serial port of MCU, -P sets the protocol

Polling interface @ curse.py:
Set networking configuration on start up, pressing enter with no input will set configuration to default wiznet setup config.
