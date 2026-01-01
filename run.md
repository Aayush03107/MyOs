# 1. CLEAN
rm *.o myos.bin

# 2. ASSEMBLE BOOTLOADER
$HOME/os-dev/cross/bin/i686-elf-as boot.s -o boot.o

# 3. COMPILE (With new folder paths)

# --- Kernel (Root) ---
$HOME/os-dev/cross/bin/i686-elf-g++ -c kernel.cpp -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils

# --- CPU Drivers ---
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/gdt.cpp -o gdt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/idt.cpp -o idt.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c cpu/pic.cpp -o pic.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils

# --- Memory Managers ---
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/pmm.cpp -o pmm.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/paging.cpp -o paging.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils
$HOME/os-dev/cross/bin/i686-elf-g++ -c memory/kheap.cpp -o kheap.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils

# --- Utilities ---
$HOME/os-dev/cross/bin/i686-elf-g++ -c utils/string.cpp -o string.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -I memory -I cpu -I utils

# 4. LINK
$HOME/os-dev/cross/bin/i686-elf-g++ -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o gdt.o idt.o pic.o pmm.o paging.o kheap.o string.o -lgcc

# 5. RUN
qemu-system-i386 -kernel myos.bin