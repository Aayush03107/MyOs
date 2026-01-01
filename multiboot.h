/* multiboot.h - The Map GRUB gives us */
#pragma once
#include <stdint.h>

/* The Multiboot Header Info */
struct multiboot_info_t {
    uint32_t flags;
    uint32_t mem_lower; // Basic memory (usually 640KB)
    uint32_t mem_upper; // Extended memory (The rest of your RAM)
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length; // Size of the memory map buffer
    uint32_t mmap_addr;   // Address of the memory map buffer
} __attribute__((packed));

/* The Memory Map Entry */
// GRUB gives us a list of these entries.
struct multiboot_memory_map_t {
    uint32_t size;
    uint64_t addr; // Start address of the block
    uint64_t len;  // Length of the block
    uint32_t type; // 1 = Available RAM, 2 = Reserved (Do not touch!)
} __attribute__((packed));