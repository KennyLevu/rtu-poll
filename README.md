// https://wiki.laptop.org/go/SDCC#Compiling_a_program
Steps:
Compile C files using sdcc and flash program with stcgal OR run make

Using Funduino Mini Wiznet 5100

The ﬁrst two ﬁles will need to be compiled separately with the commands:
sdcc -c --model-small wiz.c
sdcc -c --model-small serial.c
Then compile the source ﬁle containing the main() function and link the ﬁles together with the following command:
sdcc --model-small main.c i2c.rel lcd.rel 

ISP Flash the ihx file using stcgal:
stcgal -p COM8 -P stc89 -o cpu_6t_enabled true  main.ihx

-p sets serial port of MCU, -P sets the protocol f, LedBlink.ihx is the path to the code image


commms:
sdcc --model-small main.c
stcgal -p COM8 -P stc89 -o cpu_6t_enabled=true  main.ihx

script:
make udp IP=126.10.210.11 PORT=5000 MODE=UDP MESSAGE="0makepython test"