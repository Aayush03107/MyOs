/* pic.cpp - Programmable Interrupt Controller */
#include "io.h"
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x11  // Command: "Start Initialization Sequence"
#define ICW4_8086    0x01  // Command: "We are an 8086/88 PC"

void pic_remap() {
    // 1. Save the current masks
    // (The BIOS might have disabled some interrupts, we want to respect that)
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // 2. Start the Initialization Sequence (ICW1)
    // Tell both PICs: "Listen up, configuration data is coming!"
    outb(PIC1_COMMAND, ICW1_INIT);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT);
    io_wait();

    // 3. Set the Vector Offsets (ICW2) - THIS IS THE IMPORTANT PART
    // Master starts at 32 (0x20)
    outb(PIC1_DATA, 0x20); 
    io_wait();
    // Slave starts at 40 (0x28)
    outb(PIC2_DATA, 0x28); 
    io_wait();

    // 4. Tell them how they are connected (ICW3)
    // Tell Master: "You have a Slave attached to IRQ 2 (0000 0100)"
    outb(PIC1_DATA, 4);
    io_wait();
    // Tell Slave: "You are attached to IRQ 2"
    outb(PIC2_DATA, 2);
    io_wait();

    // 5. Set the Mode (ICW4)
    // "We are running in 8086 mode" (Not MCS-80)
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // 6. Restore the masks
    // (For now, we actually want to ENABLE interrupts, but let's just write 0)
    // Writing 0x00 to the Data port means "Unmask Everything" (Let all signals through)
    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);
}