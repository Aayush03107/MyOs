#pragma once
#include <stdint.h>
struct GDTEntry
{
    uint16_t limit_low;// lower 16 bits of the segment limit
    uint16_t base_low;// lower 16 bits of segment base address
    uint8_t base_middle;// middle 8 bits of segment base address
    uint8_t access;//access byte , determines what type of ring this segment can be used in 
    uint8_t granularity;//top 4 bits of limit+flags
    uint8_t base_high;// last 8 bits of segment base address
};
// to tell the processor where our gdt is we have to give the address of special pointer 

struct GDTPtr{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));

class GDT{
public:
    static void intialise();
};


