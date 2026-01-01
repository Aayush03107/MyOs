/* io.h - The Bridge to Hardware Ports */
#pragma once
#include <stdint.h>

/* 1. OUTB (Output Byte)
   Sends 8 bits of data to a specific hardware port.
   Usage: outb(0x21, 0xFF); // Send 0xFF to Port 0x21
*/
static inline void outb(uint16_t port, uint8_t val) {
    /* Assembly Breakdown:
       "outb %0, %1" : The instruction
       "a"(val)      : Put 'val' into the AL register (part of EAX)
       "Nd"(port)    : Put 'port' into the DX register
    */
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/* 2. INB (Input Byte)
   Reads 8 bits of data from a hardware port.
   Usage: uint8_t key = inb(0x60); // Read from Keyboard Port
*/
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    /* Assembly Breakdown:
       "=a"(ret) : Put the result (from AL) into variable 'ret'
       "Nd"(port): Put 'port' into DX register
    */
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

/* 3. IO_WAIT
   Hardware is slow. Sometimes we need to waste a tiny bit of time
   to let the chip catch up before sending the next command.
   Writes to an unused port (0x80) to burn a few microseconds.
*/
static inline void io_wait(void) {
    outb(0x80, 0);
}