/* idt.h - The Phone Book Blueprint */
#pragma once
#include <stdint.h>

/* 1. The IDT Entry 
   This matches the hardware layout exactly. 
   It tells the CPU: "When Interrupt X happens, jump to this address." */
struct IDTEntry {
    uint16_t base_low;   // Lower 16 bits of the address to jump to
    uint16_t selector;   // Kernel Code Segment Selector (0x08)
    uint8_t  zero;       // Must always be zero (Reserved)
    uint8_t  type_attr;  // Flags (Present? Ring 0? Gate Type?)
    uint16_t base_high;  // Upper 16 bits of the address to jump to
} __attribute__((packed));

/* 2. The IDT Pointer 
   Just like the GDT pointer, this tells the CPU where the table is. */
struct IDTPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* 3. The Class */
class IDT {
public:
    static void initialize();
};