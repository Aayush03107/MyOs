/* kheap.h - Updated for Phase 5 */
#pragma once
#include <stddef.h>
#include <stdint.h>

struct AllocationHeader {
    size_t size;
    bool is_free;
    AllocationHeader* next;
};

class KHeap {
public:
    static void initialize(void* start, size_t size);
    static void* allocate(size_t size);
    static void free(void* ptr);
    
    // Existing helper
    static size_t get_used_memory();

    // NEW: Getters for the 'heap' command
    static uint32_t get_start_addr();
    static uint32_t get_end_addr();
    static uint32_t get_current_addr();
};

/* --- The Engine Room (Standard Operators) --- */
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p);
void operator delete[](void* p);
void operator delete(void* p, size_t size);

/* --- YOUR CUSTOM VOCABULARY --- */
#define fresh new
#define kill  delete