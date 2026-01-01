/* pmm.cpp - Physical Memory Manager Logic */
#include "pmm.h"

// The Bitmap Array: 1024 integers to track 32,768 blocks
static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];
static uint32_t pmm_total_blocks = 0;
static uint32_t pmm_used_blocks = 0;

/* --- Private Helpers --- */

// Set bit to 1 (Mark Used)
static inline void mmap_set(int bit) {
    pmm_bitmap[bit / 32] |= (1 << (bit % 32));
}

// Unset bit to 0 (Mark Free)
static inline void mmap_unset(int bit) {
    pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Check if bit is 0 (Is Free?)
static inline bool mmap_test(int bit) {
    return pmm_bitmap[bit / 32] & (1 << (bit % 32));
}

// Find first free bit (Returns index 0 to 32768)
static int mmap_first_free() {
    for (uint32_t i = 0; i < pmm_total_blocks / 32; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) { // Optimization: Skip full buckets
            for (int j = 0; j < 32; j++) {
                int bit = 1 << j;
                if (!(pmm_bitmap[i] & bit)) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1; // Out of Memory
}

/* --- Public Functions --- */

void PMM::initialize(uint32_t total_ram_mb) {
    pmm_total_blocks = (total_ram_mb * 1024 * 1024) / PMM_BLOCK_SIZE;

    // 1. Mark EVERYTHING as used first (Safety)
    for (uint32_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    // 2. Mark actual RAM as free
    for (uint32_t i = 0; i < pmm_total_blocks; i++) {
        mmap_unset(i);
    }

    // 3. Reserve the first 2MB (Kernel + BIOS lives here)
    // 2MB = 512 Blocks
    for (uint32_t i = 0; i < 512; i++) {
        mmap_set(i);
    }
}

void* PMM::alloc_block() {
    int frame = mmap_first_free();
    
    if (frame == -1) return NULL; // No memory left

    mmap_set(frame); // Mark as used

    uint32_t addr = frame * PMM_BLOCK_SIZE;
    return (void*)addr;
}

void PMM::free_block(void* address) {
    uint32_t addr = (uint32_t)address;
    int frame = addr / PMM_BLOCK_SIZE;
    mmap_unset(frame);
}