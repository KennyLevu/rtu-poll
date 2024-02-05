// https://wiki.laptop.org/go/SDCC#Compiling_a_program
Steps:
Compile C files using sdcc

Using Funduino Mini Wiznet 5100

The ﬁrst two ﬁles will need to be compiled separately with the commands:
sdcc -c i2c.c -model-small
sdcc -c lcd.c -model-small
Then compile the source ﬁle containing the main() function and link the ﬁles together with the following command:
sdcc main.c i2c.rel lcd.rel -model-small

Alternatively, main.c can be separately compiled as well:
sdcc -c main.c
sdcc main.rel i2c.rel lcd.rel


ISP Flash the ihx file using stcgal:
stcgal -p COM8 -P stc89 -o cpu_6t_enabled true  main.ihx

-p sets serial port of MCU, -P sets the protocol f, LedBlink.ihx is the path to the code image
