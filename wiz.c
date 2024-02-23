#include <8051.h>
#include <stdint.h>
#include <stdio.h>
#include "periph.h"
#include "constant.h"

#define USE_ASM
#ifdef USE_ASM
void asm_8(uint8_t command) 
{
    command;
    __asm 
        mov A, dpl                      ; store command in accumulator
        mov r7, #0x00                   ; set counter r7 to 0
    00003$:
        cjne    r7, #0x08, 00004$       ; if r7 is not equal to 8, jump to label
        sjmp    00005$                  ; jump to end of loop label
    00004$:
                jb ACC.7, 00001$        ; if MSB in accumulator is set, jump to label
                clr     _P2_3           ; set MOSI LOW if MSB is cleared 
                sjmp      00002$        ; jump to clock label
    00001$:
                setb	_P2_3           ; set MOSI HIGH
    00002$:
                setb     _P2_1          ; CLOCK HIGH LOW
                clr      _P2_1
                rlc      A               ; rotate accumulator left 1
        inc     r7                      ; increment counter r7
        sjmp    00003$                  ; jump to top of loop
    00005$:
    __endasm;
}

void asm_16(uint16_t command) 
{   
    command;
    __asm 
        mov A, dph                      ; store high byte of command in accumulator
        mov r7, #0x00                   ; set counter r7 to 0
    00006$:
        cjne    r7, #0x08, 00007$       ; if r7 is not equal to 8, jump to label
        sjmp    00010$                  ; jump to to high byte loop
    00007$:
                jb ACC.7, 00008$        ; if MSB in accumulator is set, jump to label
                clr     _P2_3           ; set MOSI LOW if MSB is cleared 
                sjmp      00009$        ; jump to clock label
    00008$:
                setb	_P2_3           ; set MOSI HIGH
    00009$:
                setb     _P2_1          ; CLOCK HIGH LOW
                clr      _P2_1
                rlc      A               ; rotate accumulator left 1
        inc     r7                      ; increment counter r7
        sjmp    00006$                  ; jump to top of loop
    
    00010$:
        mov A, dpl                      ; store lower byte of command in accumulator
        mov r7, #0x00                   ; reset counter r7
    00011$:
        cjne    r7, #0x08, 00012$       ; if r7 is not equal to 8, jump to label
        sjmp    00015$                  ; jump to end of loop label
    00012$:
                jb ACC.7, 00013$        ; if MSB in accumulator is set, jump to label
                clr     _P2_3           ; set MOSI LOW if MSB is cleared 
                sjmp      00014$        ; jump to clock label
    00013$:
                setb	_P2_3           ; set MOSI HIGH
    00014$:
                setb     _P2_1          ; CLOCK HIGH LOW
                clr      _P2_1
                rlc      A               ; rotate accumulator left 1
        inc     r7                      ; increment counter r7
        sjmp    00011$                  ; jump to top of loop
    00015$:

    __endasm;
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