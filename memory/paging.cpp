/* paging.cpp */
#include "paging.h"
#include "pmm.h"
#include "io.h" // For debugging if needed

// We need an array for the Directory (1024 entries)
// We must align it to 4096 bytes (Page Aligned) so the CPU can find it.
// usage of __attribute__((aligned(4096))) is critical!
page_directory_entry_t page_directory[1024] __attribute__((aligned(4096)));

// We also need one Page Table to cover the first 4MB (0 - 4MB)
page_table_entry_t first_page_table[1024] __attribute__((aligned(4096)));

// Assembly helper to load the directory address into CR3 register
extern "C" void load_page_directory(uint32_t*);
// Assembly helper to set the Paging Enable bit in CR0
extern "C" void enable_paging();

void Paging::initialize() {
    // 1. Clear the Directory (Mark everything as "Not Present")
    for (int i = 0; i < 1024; i++) {
        // Attribute 2: Read/Write (bit 1 = 1)
        // Attribute 0: Present (bit 0 = 0) -> NOT PRESENT yet
        page_directory[i] = 0x00000002;
    }

    // 2. Identity Map the first 4MB
    // We want Virtual 0x000000 -> Physical 0x000000
    // We want Virtual 0x001000 -> Physical 0x001000
    // ... up to 4MB.
    
    for (int i = 0; i < 1024; i++) {
        // Address calculation: i * 4096 (0, 4096, 8192...)
        uint32_t address = i * 4096;
        
        // Entry = Address | 3 (Binary 011)
        // Bit 0 (Present) = 1
        // Bit 1 (Read/Write) = 1
        first_page_table[i] = address | 3; 
    }

    // 3. Put this Table into the Directory
    // The first entry (Index 0) of the directory covers 0-4MB.
    // We point it to our table.
    // We cast to uint32_t because the hardware just wants the address.
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    // 4. Tell the CPU where the directory is
    load_page_directory(page_directory);

    // 5. Flip the switch!
    enable_paging();
}