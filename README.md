Resource: https://wiki.laptop.org/go/SDCC#Compiling_a_program

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
