#include <8051.h>
#include <stdint.h>
#include <stdio.h>
#include "periph.h"
#include "constant.h"

#define USE_ASM
#ifdef USE_ASM
void asm_8(uint8_t command) 
{
    __asm 
        mov A, dpl
    __endasm;
    for (uint8_t i = 0; i < 8; i++) {
            __asm
                jb ACC.7, 00001$
                clr     _P2_3
                sjmp      00002$
                00001$:
                setb	_P2_3
                00002$:
                setb     _P2_1
                clr      _P2_1
                rl A
            __endasm;
        // command = command << 1;
    }
    // for (uint8_t i = 0; i < 16; i++) {
    //         if (command & 0x8000) {
    //             P2_3 = 1;
    //         }
    //         else {
    //             P2_3 = 0;
    //         }
    //         P2_1 = 1;
    //         command = command << 1;
    //         P2_1 = 0;
    // }

}

void asm_16(uint16_t command) 
{   
    
    for (uint8_t i = 0; i < 16; i++) {
        // __asm
        //     jb ACC.7, 00001$
        //     clr     _P2_3
        //     sjmp      00002$
        //     00001$:
        //     setb	_P2_3
        //     00002$:
        //     setb     _P2_1
        //     clr      _P2_1
        //     rl A
        // __endasm;
            if (command & 0x8000) {
                P2_3 = 1;
            }
            else {
                P2_3 = 0;
            }
            P2_1 = 1;
            command = command << 1;
            P2_1 = 0;
    }
    // for (uint8_t i = 0; i < 16; i++) {
    //         if (command & 0x8000) {
    //             P2_3 = 1;
    //         }
    //         else {
    //             P2_3 = 0;
    //         }
    //         P2_1 = 1;
    //         command = command << 1;
    //         P2_1 = 0;
    // }
}

// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) 
{
    CS = LOW;
    // uint8_t command = OP_WRITE;
    asm_8(OP_WRITE);

    asm_16(addr);

    asm_8(data);
    CS = HIGH;
}

// OP Read Instruction
uint8_t wiz_read(uint16_t addr) 
{
    CS = LOW;
    uint8_t byte = 0; // hold output data
    // uint8_t command = OP_READ;
    asm_8(OP_READ);

    asm_16(addr);

    for (uint8_t i = 0; i < 8; i++) {
        CLK = HIGH;
        // delay10();
        // delay_us(2);
        byte = byte << 1; // create room for new bit
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
    }

    CS = HIGH;
    return byte;
}
#else
    // write bytes to MOSI 
    void cmdout_16(uint16_t command) 
    {
        for (uint8_t i = 0; i < BYTE_16; i++) {
            if (command & MSK_16) {
                MOSI = HIGH;
            }
            else {
                MOSI = LOW;
            }
            // delay10();
            CLK = HIGH;
            command = command << 1;
            CLK = LOW;
        }
    }
    void cmdout_8(uint8_t command) 
    {
        for (uint8_t i = 0; i < BYTE_8; i++) {
            if (command & MSK_8) {
                MOSI = HIGH;
            }
            else {
                MOSI = LOW;
            }
            // delay10();
            CLK = HIGH;
            command = command << 1;
            CLK = LOW;
        }
    }


// OP Write Instruction
void wiz_write(uint16_t addr, uint8_t data) 
{
    CS = LOW;
    // uint8_t command = OP_WRITE;
    cmdout_8(OP_WRITE);

    cmdout_16(addr);

    cmdout_8(data);
    CS = HIGH;
}

// OP Read Instruction
uint8_t wiz_read(uint16_t addr) 
{
    CS = LOW;
    uint8_t byte = 0; // hold output data
    // uint8_t command = OP_READ;
    cmdout_8(OP_READ);

    cmdout_16(addr);

    for (uint8_t i = 0; i < 8; i++) {
        CLK = HIGH;
        // delay10();
        // delay_us(2);
        byte = byte << 1; // create room for new bit
        byte = byte | MISO; // shift in MSB
        CLK = LOW;
    }

    CS = HIGH;
    return byte;
}
#endif


// Read data buffer from wiznet address
void wiz_read_buf(uint16_t addr, uint16_t len, uint8_t *buffer) 
{
    for (int i = 0; i < len; i++) {
        buffer[i] = wiz_read(addr + i);
    }
}

// write to address from data buffer
void wiz_write_buf(uint16_t addr, uint16_t len, uint8_t *buffer) 
{
    for (int i = 0; i < len; i++) {
        wiz_write(addr + i, buffer[i]);
    }
}