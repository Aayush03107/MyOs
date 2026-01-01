/* idt.cpp */
#include "idt.h"
extern "C" void keyboard_handler_stub();
// We need 256 entries (0-255)
// The CPU has 256 possible interrupt slots.
IDTEntry idt_entries[256];
IDTPtr   idt_ptr;

// Helper to set an entry
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    
    idt_entries[num].selector  = sel;
    idt_entries[num].zero      = 0;
    idt_entries[num].type_attr = flags; // 0x8E = Present, Ring 0, Interrupt Gate
}

// Defined in Assembly (we will write this later)
extern "C" void idt_flush(uint32_t);

void IDT::initialize() {
    idt_ptr.limit = (sizeof(IDTEntry) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // 1. Clear the IDT (Set everything to zero for now)
    // In a real OS, we would set defaults here to catch crashes.
    // For now, we leave them blank. If an interrupt happens, the CPU will Triple Fault.
    // This is expected until we write the handlers!
    for (int i=0; i<256; i++) {
        // base=0, selector=0x08 (Kernel Code), flags=0x8E
        idt_set_gate(i, 0, 0x08, 0x8E); 
    }
    idt_set_gate(33, (uint32_t)keyboard_handler_stub, 0x08, 0x8E);
    // 2. Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}