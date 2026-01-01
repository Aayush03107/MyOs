/* gdt.cpp - The "Future-Proof" 5 Entry Version */
#include "gdt.h"

GDTEntry gdt_entries[5];
GDTPtr   gdt_ptr;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access      = access;
}

extern "C" void gdt_flush(uint32_t);

void GDT::intialise() {
    // CHANGED: Size is now 5 entries
    gdt_ptr.limit = (sizeof(GDTEntry) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // 0. Null Descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1. Kernel Code (Ring 0) - Offset 0x08
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2. Kernel Data (Ring 0) - Offset 0x10
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 3. User Code (Ring 3) - Offset 0x18
    // Access 0xFA = 1111 1010b (Present, Ring 3, Executable, Readable)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 4. User Data (Ring 3) - Offset 0x20
    // Access 0xF2 = 1111 0010b (Present, Ring 3, Writable)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gdt_ptr);
}