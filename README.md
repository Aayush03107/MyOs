# MyOS: A 32-bit C++ Operating System Kernel

Welcome to **MyOS**, a custom operating system kernel written from scratch in C++ and Assembly. This project demonstrates low-level systems programming concepts including protected mode booting, direct hardware manipulation, memory management, and a custom shell.

## 🚀 Features

### Phase 1: The Boot Process
- **Multiboot Compliant:** Boots via GRUB/QEMU using a custom assembly bootloader (`boot.s`).
- **32-bit Protected Mode:** Sets up the GDT (Global Descriptor Table) to transition the CPU from Real Mode.

### Phase 2: Hardware Drivers (`cpu/`)
- **VGA Text Mode:** Custom driver to write directly to video memory (`0xB8000`) with color support.
- **IDT (Interrupt Descriptor Table):** Handles CPU exceptions and hardware interrupts.
- **PIC (Programmable Interrupt Controller):** Remapped to avoid conflicts with CPU exceptions.
- **PS/2 Keyboard:** Translates raw scancodes into ASCII characters.

### Phase 3: Memory Management (`memory/`)
- **PMM (Physical Memory Manager):** Bitmapped allocator for tracking 4KB physical RAM pages.
- **Paging:** Enables virtual memory and identity mapping.
- **Kernel Heap:** Custom dynamic memory allocator (`new`/`delete` operators) implemented from scratch.

### Phase 4: Standard Library (`utils/`)
- **Custom STL:** Implementation of `String` and `Vector` classes without `libstdc++`.
- **Utils:** String manipulation (`strcmp`, `strlen`) and memory helpers.

### Phase 5: The Shell
- **Command Line Interface:** Interactive shell with a command history buffer.
- **System Commands:**
  - `mem`, `heap`: View detailed memory statistics.
  - `ticks`, `uptime`: View CPU cycle counts and timing.
  - `panic`: Tests the kernel panic handler.
  - `reboot`, `halt`: Direct hardware control.

---

## 📂 Project Structure

```text
myos/
├── boot.s          # Assembly Bootloader
├── kernel.cpp      # Kernel Entry & Shell Loop
├── linker.ld       # Linker Script
│
├── cpu/            # Hardware Drivers
│   ├── gdt.cpp     # Global Descriptor Table
│   ├── idt.cpp     # Interrupt Descriptor Table
│   └── pic.cpp     # Interrupt Controller
│
├── memory/         # Memory Managers
│   ├── pmm.cpp     # Physical Page Allocator
│   ├── paging.cpp  # Virtual Memory
│   └── kheap.cpp   # Kernel Heap (kmalloc/free)
│
└── utils/          # Custom C++ Library
    ├── string.cpp  # String class & helpers
    └── vector.h    # Dynamic array implementation
🛠️ How to Build & Run
Prerequisites
GCC Cross Compiler (i686-elf-g++)

QEMU (x86 System Emulator)

Binutils (Assembler & Linker)

Compilation
This project compiles without a standard library (-nostdlib). Run the following command sequence to build the ISO binary:

Bash

# 1. Clean old files
rm *.o myos.bin

# 2. Assemble Bootloader
$HOME/os-dev/cross/bin/i686-elf-as boot.s -o boot.o

# 3. Compile C++ Kernel Modules (Includes cpu, memory, and utils)
$HOME/os-dev/cross/bin/i686-elf-g++ -c kernel.cpp -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/gdt.cpp -o gdt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/idt.cpp -o idt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/pic.cpp -o pic.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/pmm.cpp -o pmm.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/paging.cpp -o paging.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/kheap.cpp -o kheap.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c utils/string.cpp -o string.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils

# 4. Link
$HOME/os-dev/cross/bin/i686-elf-g++ -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o gdt.o idt.o pic.o pmm.o paging.o kheap.o string.o -lgcc
Running in QEMU
Bash

qemu-system-i386 -kernel myos.bin
Author: Aayush Sharma

Built for educational purposes to understand OS internals.