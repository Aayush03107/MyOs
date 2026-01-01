/* kheap.cpp - Memory Management + Status Accessors */
#include "kheap.h"

// 1. GLOBAL COUNTER
size_t memory_used = 0;

/* --- KHeap Implementation --- */
uint32_t heap_start_addr;
uint32_t heap_end_addr;
uint32_t heap_curr_addr;

void KHeap::initialize(void* start, size_t size) {
    heap_start_addr = (uint32_t)start;
    heap_end_addr = heap_start_addr + size;
    heap_curr_addr = heap_start_addr;
}

void* KHeap::allocate(size_t size) {
    // 4-byte alignment
    if (heap_curr_addr % 4 != 0) {
        heap_curr_addr += 4 - (heap_curr_addr % 4);
    }
    
    uint32_t addr = heap_curr_addr;
    heap_curr_addr += size;

    if (heap_curr_addr >= heap_end_addr) {
        return 0; // Out of memory
    }
    return (void*)addr;
}

void KHeap::free(void* ptr) {
    (void)ptr; // No-op for now (Bump allocator)
}

// Existing: Get used memory
size_t KHeap::get_used_memory() {
    return memory_used;
}

// NEW: Implement the getters
uint32_t KHeap::get_start_addr() { return heap_start_addr; }
uint32_t KHeap::get_end_addr() { return heap_end_addr; }
uint32_t KHeap::get_current_addr() { return heap_curr_addr; }

/* --- C++ OPERATORS --- */
void* operator new(size_t size) {
    memory_used += size;
    return KHeap::allocate(size);
}

void* operator new[](size_t size) {
    memory_used += size;
    return KHeap::allocate(size);
}

void operator delete(void* p) { KHeap::free(p); }
void operator delete(void* p, size_t size) { (void)size; KHeap::free(p); }
void operator delete[](void* p) { KHeap::free(p); }
void operator delete[](void* p, size_t size) { (void)size; KHeap::free(p); }