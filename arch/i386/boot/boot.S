/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     arch/i386/boot/boot.S
 * Description:   Starting point for the kernel. Defines the multiboot header
 *		  and calls the main kernel function. Also sets a new GDT and
 *		  allocates space for the idt
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 ********************************************************************/

	
/* The system starts at address 0x100000 (1MB). We are in protected mode
 * with the A20 line enabled and a GDT which we cannot trust. Interrupts
 * are as yet disabled. Therefore we setup a new GDT and a stand in IDT.
 * We then enter the main kernel.
 * That is about it.
 */
	
.section .text

.global _start, __idt, __gdt
.global kernel_pg_dir, pg_table1, pg_table2, pg_table3
.global init_gdt	

.comm __kernel_virt_addr, 0
.comm __kernel_load_addr, 0

_start:
	
/* Loading the physical address of the stack end into the esp register */
	leal __kernel_load_addr, %esp  
	leal _stack, %ecx
	addl $8192, %ecx
	leal __kernel_virt_addr, %edx
	subl %edx, %ecx
	addl %ecx, %esp
	movl $0, %ebp


	/* Call init_mm with the necessary arguments */

	pushl %ebx
	pushl %eax
	call init_mm

	/* Setup the new gdt */
	lgdt _gdt_ptr
	jmp $0x10, $_label  /*A far jump to reset CS:EIP */
_label:
	movw $0x8, %cx
	movw %cx, %ss
	movw %cx, %es
	movw %cx, %gs
	movw %cx, %ds
	movw %cx, %fs


	/* Setup the IDT */				
	call init_idt

	/* Update the stack pointer to sync with the virtual address space */
	pushl %ecx
 	pushl %edx
	leal __kernel_virt_addr, %edx
	leal __kernel_load_addr, %ecx
	subl %ecx, %esp
	addl %edx, %esp
 	popl %edx
	popl %ecx 

	/* Start the Kernel */
	call kstart	 


	/*  Keep the data on screen. */	
_loop:			
	nop	
	jmp _loop

_mb_header_:
	.align 4		#  The multiboot header
	.long 0x1badb002
	.long 0x00000000
	.long -(0x1badb002 + 0x00000000) 



	# Initializing the IDT

init_idt:
	pushl %ebp
	movl %esp, %ebp
	call fill_idt


	lidt _idt_ptr

	leave
	ret



/* Fill the idt.
 * BIG FAT NOTE : The interrupt gates that are filled in the idt assume that
 * code segment for the interrupt service routine is at the 3rd slot of the GDT
 * ie.. offset 0x10 (taking into account the mandatory null entry at the beginning
 *
 * So take this into account if you make any changes to the gdt.
 */
fill_idt:
	pushl %ebp
	movl %esp, %ebp
	
	lea dummy_int_handler, %eax
	movw $0x10, %bx
	shl $16, %ebx
	movw %ax, %bx
	movw $0x8e00, %ax	
		
	movl $256, %ecx
	lea __idt, %edx

1:	movl %ebx, (%edx)
	movl %eax, 4(%edx)
	addl $8, %edx
	decl %ecx
	loop 1b	
	
	leave
	ret
	
/* A stand in interrupt handler which cries out  
 * "Unhandled Interrupt!\n"!!
 */
dummy_int_handler:
	pushl %ebp
	movl %esp, %ebp
	
	cli
	
	pushl $ignore_msg
	call printf	

	addl $4, %esp

	movb $0x20, %al
	outb %al, $0x20 
	
	sti
	leave
	iret


display_char:
	pushl %ebp
	movl %esp, %ebp
	movl $0xB8000, %esi
	movl %edi, (%esi)
	movl $7, %edi
	incl %esi
	movl %edi , (%esi)

	leave
	ret


.section .data
	
.align 8
__gdt:
	.long 0, 0				  # Null Entry.	 
	.byte 0xff, 0xff, 0, 0, 0, 0x93, 0xcf, 0  # Data/stack Segment
	.byte 0xff, 0xff, 0, 0, 0, 0x9d, 0xcf, 0  # Code Segment

_idt_ptr:	.word 256*8 - 1
		.long __idt

_gdt_ptr:	.word 3*8 - 1		
 		.long __gdt


	
	# Message strings
halt_msg:
	.asciz "\nIt is now safe to turn off your computer  :) \n"

ignore_msg:	.asciz "Unhandled Interrupt!\n"



/* BSS Section */
.section .bss

.align 4096
kernel_pg_dir:	.fill 4096,1,0  /* The kernel page directory */

.align 4096
pg_table1 :	.fill 4096,1,0  /* Some page tables that we need before we
				 * can enable paging */

.align 4096
pg_table2 :	.fill 4096,1,0
	
.align 4

_stack : .fill 8192, 1, 0

.align 8	
__idt:	.fill 256,8,0		# Allocate space for 256 Interrupt handlers

