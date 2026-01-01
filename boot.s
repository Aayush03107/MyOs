/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/
.section .text
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the
	machine.
	*/

	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp
	/* Push the pointer to the Multiboot Info (stored in EBX) onto the stack */
	push %ebx

	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/

	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far), so the alignment has thus been
	preserved and the call is well defined.
	*/
	call kernel_main

	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
1:	hlt
	jmp 1b

/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start

/* --- Add this to the bottom of boot.s --- */

/* 1. Make this label visible to C++ */
.global gdt_flush
.type gdt_flush, @function

gdt_flush:
	/* 2. Grab the pointer */
	/* When C++ calls a function, it pushes arguments onto the Stack. */
	/* 4(%esp) is where the first argument (your gdt_ptr address) lives. */
	mov 4(%esp), %eax
	
	/* 3. Load the GDT */
	/* This is the magic instruction. It tells the CPU: */
	/* "Read the limit and base from the address in EAX and use it as the new GDT." */
	lgdt (%eax)

	/* 4. Update the Data Segment Registers */
	/* Now that we have a new GDT, we must tell the CPU to use Entry #2 (0x10) for data. */
	/* Why 0x10? Each entry is 8 bytes. Entry 0 is 0x00, Entry 1 is 0x08, Entry 2 is 0x10. */
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	/* 5. Update the Code Segment Register (CS) */
	/* We cannot simply 'mov' into CS. The CPU forbids it. */
	/* We must use a "Far Jump" (ljmp) to force the CPU to update CS to Entry #1 (0x08). */
	jmp $0x08, $.flush

.flush:
	/* 6. Return to C++ */
	/* We are now fully running in the new GDT! */
	ret
	
/* --- Add this to the bottom of boot.s --- */

.global idt_flush
.type idt_flush, @function
idt_flush:
	mov 4(%esp), %eax  /* Get the pointer */
	lidt (%eax)        /* Load the IDT */
	ret
/* --- Add this to the bottom of boot.s --- */

/* 1. Make the wrapper visible to C++ */
.global keyboard_handler_stub

/* 2. Declare that the C++ function exists somewhere */
.extern keyboard_handler_main

keyboard_handler_stub:
    /* A. Save the CPU State */
    pusha               /* Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI */
    
    /* B. Call the C++ Handler */
    call keyboard_handler_main
    
    /* C. Restore the CPU State */
    popa                /* Pop everything back exactly as it was */
    
    /* D. Return from Interrupt */
    iret
/* --- Add to the bottom of boot.s --- */

.global load_page_directory
.type load_page_directory, @function
load_page_directory:
	push %ebp
	mov %esp, %ebp
	
	/* Get the argument (pointer to directory) from stack */
	mov 8(%ebp), %eax
	
	/* Move it into Control Register 3 (CR3) */
	mov %eax, %cr3
	
	mov %ebp, %esp
	pop %ebp
	ret

.global enable_paging
.type enable_paging, @function
enable_paging:
	push %ebp
	mov %esp, %ebp
	
	/* Read CR0 */
	mov %cr0, %eax
	
	/* Set Bit 31 (The "PG" Paging Bit) */
	/* 0x80000000 = 1000...0000 binary */
	or $0x80000000, %eax
	
	/* Write it back to CR0 to enable */
	mov %eax, %cr0
	
	mov %ebp, %esp
	pop %ebp
	ret