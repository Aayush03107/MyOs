/* pmm.h - Physical Memory Manager Header */
#pragma once
#include <stdint.h>
#include <stddef.h>

// 1. Define Block Size (4KB)
static const uint32_t PMM_BLOCK_SIZE = 4096;

// 2. Define Bitmap Size
// 128MB RAM / 4KB Blocks = 32,768 Blocks.
// 32,768 Blocks / 32 bits per integer = 1024 integers.
static const uint32_t PMM_BITMAP_SIZE = 1024;

class PMM {
public:
    // Initialize the manager with the total amount of RAM in MB
    static void initialize(uint32_t total_ram_mb);

    // Ask for a free block (Returns address like 0x200000)
    static void* alloc_block();

    // Return a block when done
    static void free_block(void* address);
};