#include <8051.h>
#include <stdint.h>
#include "periph.h"
#include "constant.h"

#define USE_ASM
#ifdef USE_ASM
void asmout_8(uint8_t command) 
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        asm volatile ( // tells compiler that the following is assembly code, volatile flags the code to prevent opimization 
            "mov     r1, %[cmd]       \n"  // Move the command to a register
            "\n"
            "srl     r1, #7           \n"  // Shift the command to get the most significant bit
            "jc      set_bit          \n"  // If carry flag is set, jump to set_bit label
            "clr     P2.3             \n"  // Clear P2.3 if the MSB is 0
            "sjmp    next_bit         \n"  // Jump to next_bit label
            "set_bit:                 \n"  // Label to set P2.3 if MSB is 1
            "setb    P2.3             \n"  // Set P2.3 if the MSB is 1
            "next_bit:                \n"  // Label to proceed to the next bit
            "setb    P2.1             \n"  // Set P2.1
            "clr     P2.1             \n"  // Clear P2.1
            : // No output 
            : [cmd] "r" (command) // Input operand - command
            : "r1", "cc" // clobber list - register r1 and condition codes
        );
        command = command << 1; // Shift the command to the left by one bit
    }
}

void asm_16(uint16_t command) 
{
    uint8_t i;
    for (i = 0; i < 16; i++) {
        asm volatile ( 
            "mov     r1, %[cmd]       \n" 
            "srl     r1, #15          \n"
            "jc      set_bit          \n"
            "clr     P2.3             \n"
            "sjmp    next_bit         \n"
            "set_bit:                 \n"
            "setb    P2.3             \n"
            "next_bit:                \n"
            "setb    P2.1             \n"
            "clr     P2.1             \n"
            :
            : [cmd] "r" (command)
            : "r1", "cc"
        );
        command = command << 1;
    }
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