/* paging.h - Virtual Memory Manager */
#pragma once
#include <stdint.h>

// 1. Page Directory Entry (The Chapter)
// This points to a Page Table.
// Bit 0: Present (1 = Yes)
// Bit 1: Read/Write (1 = Writeable)
// Bit 2: User/Supervisor (0 = Kernel Only)
// Bits 12-31: Physical Address of the Page Table
typedef uint32_t page_directory_entry_t;

// 2. Page Table Entry (The Page)
// This points to a 4KB Physical Frame.
// Same flags as above.
typedef uint32_t page_table_entry_t;

// 3. The Manager Class
class Paging {
public:
    // Sets up the 1:1 map for the first 4MB and enables paging
    static void initialize();
};