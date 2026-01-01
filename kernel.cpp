/* kernel.cpp - Phase 5c: The Complete Suite */
//#include <cstring>
#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "idt.h"
#include "io.h"
#include "multiboot.h"
#include "pmm.h"
#include "paging.h"
#include "kheap.h"
#include "string.h"
#include "vector.h"
#include "keyboard.h" 

/* --- C++ SUPPORT --- */
void* __dso_handle = 0;
extern "C" void __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {
    (void)destructor; (void)arg; (void)dso;
}
extern "C" int strcmp(const char *s1, const char *s2);

// Ensure 'memory_used' is not static in kheap.cpp!
extern size_t memory_used; 

void pic_remap();

/* --- HELPERS --- */
static inline uint64_t rdtsc() {
    uint64_t ret;
    asm volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}

static inline uint32_t read_cr3() {
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void sys_reboot() {
    uint8_t temp;
    asm volatile ("cli"); 
    do {
        temp = inb(0x64);
        if (temp & 1) inb(0x60);
    } while (temp & 2);
    outb(0x64, 0xFE);
    asm volatile ("hlt");
}

/* --- VGA TERMINAL --- */
const size_t VGA_width = 80;
const size_t VGA_height = 25;
uint16_t* video_memory = (uint16_t*) 0xB8000;

enum class Color : uint8_t {
    Black = 0, Blue = 1, Green = 2, Cyan = 3, Red = 4,
    Magenta = 5, Brown = 6, LightGrey = 7, DarkGrey = 8,
    LightBlue = 9, LightGreen = 10, LightCyan = 11,
    LightRed = 12, LightMagenta = 13, LightBrown = 14, White = 15,
};

static inline uint16_t make_vga_entry(char c , Color foreground, Color background) {
    uint8_t color_byte = (uint8_t)foreground | ((uint8_t)background << 4);
    return (uint16_t)c | ((uint16_t)color_byte << 8);
}

class Terminal {
    static size_t terminal_row;
    static size_t terminal_column;
    static uint8_t terminal_color;     
    
    static void update_cursor() {
        uint16_t pos = terminal_row * VGA_width + terminal_column;
        outb(0x3D4, 0x0F); outb(0x3D5, (uint8_t)(pos & 0xFF));
        outb(0x3D4, 0x0E); outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    }

    public:   
        static void scroll(){
            for(size_t y = 0 ; y < VGA_height-1 ; y++){
                for(size_t x = 0 ; x < VGA_width; x++){
                    video_memory[y*VGA_width+x] = video_memory[(y+1)*VGA_width+x];
                }
            }
            uint16_t blank = (uint16_t)' ' | ((uint16_t)terminal_color << 8);
            for (size_t x = 0; x < VGA_width; x++) video_memory[(VGA_height-1)*VGA_width + x] = blank;
        }

        static void intialise(){
            terminal_row = 0; terminal_column = 0;
            terminal_color = (uint8_t)Color::White | ((uint8_t)Color::Black << 4);
            for(size_t i=0; i<VGA_width*VGA_height; i++) video_memory[i] = make_vga_entry(' ', Color::Black, Color::Black);
            update_cursor();
        }

        static void clear() {
            for(size_t i=0; i<VGA_width*VGA_height; i++) video_memory[i] = make_vga_entry(' ', Color::Black, Color::Black);
            terminal_row = 0; terminal_column = 0;
            update_cursor();
        }

        static void put_char(char c){
            if(c == '\n'){ terminal_column = 0; terminal_row++; } 
            else if (c == '\b') {
                if (terminal_column > 0) {
                    terminal_column--;
                    video_memory[terminal_row * VGA_width + terminal_column] = make_vga_entry(' ', Color::White, Color::Black);
                }
            } else {
                video_memory[terminal_row * VGA_width + terminal_column] = (uint16_t)c | ((uint16_t)terminal_color << 8);
                terminal_column++;
                if(terminal_column == VGA_width){ terminal_column = 0; terminal_row++; }
                if(terminal_row >= VGA_height){ scroll(); terminal_row = VGA_height-1; }
            }
            update_cursor();
        }

        static void print(const char* str) {
            for (size_t i = 0; str[i] != '\0'; i++) put_char(str[i]);
        }
        
        static void set_color(Color fg, Color bg) {
            terminal_color = (uint8_t)fg | ((uint8_t)bg << 4);
        }
};

size_t Terminal::terminal_row;
size_t Terminal::terminal_column;
uint8_t Terminal::terminal_color;
String* input_buffer = nullptr; 

/* --- PRINTERS --- */
void print_dec(uint32_t n) {
    if (n == 0) { Terminal::put_char('0'); return; }
    char buffer[32]; int i = 0;
    while (n > 0) { buffer[i] = (n % 10) + '0'; n /= 10; i++; }
    while (--i >= 0) { Terminal::put_char(buffer[i]); }
}

void print_hex(uint32_t n) {
    Terminal::print("0x");
    char hex_chars[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        Terminal::put_char(hex_chars[(n >> i) & 0xF]);
    }
}

/* --- COMMAND HANDLER --- */
extern "C" void keyboard_handler_main() {
    uint8_t status = inb(0x64); (void)status; 
    uint8_t scancode = inb(0x60);
    char letter = scancode_to_ascii(scancode);

    if (input_buffer == nullptr) { outb(0x20, 0x20); return; }

    if (letter != 0) { 
        if (letter == '\n') {
            Terminal::print("\n");
            
            if (input_buffer->len() > 0) {
                const char* cmd = input_buffer->c_str();

                /* --- SYSTEM COMMANDS --- */
                if (strcmp(cmd, "help") == 0) {
                    Terminal::print("General Commands:\n");
                    Terminal::print("  info      - OS Details & Credits\n");
                    Terminal::print("  version   - Kernel Version\n");
                    Terminal::print("  echo [x]  - Print [x] to screen\n");
                    Terminal::print("  clear     - Clear the screen\n");
                    Terminal::print("  reboot    - Restart the system\n");
                    Terminal::print("  halt      - Halt the CPU\n");
                    Terminal::print("\nDiagnostics:\n");
                    Terminal::print("  mem       - Physical Memory Stats\n");
                    Terminal::print("  heap      - Heap Allocator Stats\n");
                    Terminal::print("  paging    - Paging Control Register\n");
                    Terminal::print("  ticks     - CPU Cycle Count\n");
                    Terminal::print("  uptime    - System Uptime\n");
                    Terminal::print("  panic     - Test Kernel Panic\n");
                } 
                else if (strcmp(cmd, "info") == 0) {
                    Terminal::print("Shellos v1.0\nAuthor: Aayush Sharma\nBuilt with C++ & QEMU\n");
                }
                else if (strcmp(cmd, "version") == 0) {
                    Terminal::print("Kernel v0.0 \n");
                }
                else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ') {
                    Terminal::print(cmd + 5); Terminal::print("\n");
                }
                else if (strcmp(cmd, "clear") == 0) {
                    Terminal::clear();
                }
                else if (strcmp(cmd, "reboot") == 0) {
                    Terminal::print("Rebooting...\n");
                    sys_reboot();
                }
                else if (strcmp(cmd, "halt") == 0) {
                    Terminal::print("System Halted.\n");
                    asm volatile("cli; hlt");
                }

                /* --- DIAGNOSTIC COMMANDS --- */
                else if (strcmp(cmd, "mem") == 0) {
                    uint32_t total_pages = 32768; 
                    uint32_t used_pages = KHeap::get_used_memory() / 4096 + 1024; 
                    uint32_t free_pages = total_pages - used_pages;

                    Terminal::print("Total pages: "); print_dec(total_pages); Terminal::print("\n");
                    Terminal::print("Used pages:  "); print_dec(used_pages); Terminal::print("\n");
                    Terminal::print("Free pages:  "); print_dec(free_pages); Terminal::print("\n");
                    Terminal::print("Page size:   4096 bytes\n");
                }
                else if (strcmp(cmd, "heap") == 0) {
                    uint32_t start = KHeap::get_start_addr();
                    uint32_t end   = KHeap::get_end_addr();
                    size_t used    = KHeap::get_used_memory();
                    size_t total   = end - start;
                    size_t free    = total - used;

                    Terminal::print("Heap start:  "); print_hex(start); Terminal::print("\n");
                    Terminal::print("Heap end:    "); print_hex(end);   Terminal::print("\n");
                    Terminal::print("Used:        "); print_dec(used / 1024); Terminal::print(" KB\n");
                    Terminal::print("Free:        "); print_dec(free / 1024); Terminal::print(" KB\n");
                }
                else if (strcmp(cmd, "paging") == 0) {
                    uint32_t cr3 = read_cr3();
                    Terminal::print("Paging:      enabled\n");
                    Terminal::print("CR3:         "); print_hex(cr3); Terminal::print("\n");
                    Terminal::print("Higher-half: no\n");
                }
                else if (strcmp(cmd, "ticks") == 0) {
                    uint64_t ticks = rdtsc() / 1000000; 
                    Terminal::print("Timer ticks since boot: "); print_dec((uint32_t)ticks); Terminal::print("\n");
                }
                else if (strcmp(cmd, "uptime") == 0) {
                    uint64_t seconds = (rdtsc() / 2000000000); 
                    Terminal::print("Uptime: "); print_dec((uint32_t)seconds); Terminal::print(" seconds\n");
                }
                else if (strcmp(cmd, "panic") == 0) {
                    Terminal::set_color(Color::White, Color::Red);
                    Terminal::print("\nKernel panic: manual trigger\n");
                    asm volatile("cli; hlt");
                }

                else {
                    Terminal::print("Unknown command. Type 'help'.\n");
                }

                input_buffer->clear(); 
            }
            Terminal::print("> ");
        }
        else if (letter == '\b') {
            if (input_buffer->len() > 0) { Terminal::put_char('\b'); input_buffer->pop_back(); }
        }
        else {
            char temp[2] = {letter, '\0'}; Terminal::print(temp); input_buffer->append(letter); 
        }
    }
    outb(0x20, 0x20); 
}

/* --- KERNEL MAIN --- */
extern "C" void kernel_main(multiboot_info_t* mboot_ptr){
    Terminal::intialise();
    GDT::intialise();
    IDT::initialize();
    pic_remap();
    
    uint32_t total_mb = 128;
    if (mboot_ptr->flags & 1) { total_mb = (mboot_ptr->mem_upper + 1024) / 1024; }
    PMM::initialize(total_mb);
    Paging::initialize();
    
    void* heap_start = PMM::alloc_block();
    KHeap::initialize(heap_start, 4096);
    input_buffer = fresh String();

    Terminal::print("Welcome to MyOS Shell! Type 'help' for commands.\n");
    Terminal::print("> ");
    
    asm volatile("sti");
    while(1) { asm volatile("hlt"); }
}