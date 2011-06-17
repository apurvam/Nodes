/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     arch/i386/kernel/i8259.S
 * Description:   Initialize the PIC (IC 8259)
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

/* Note : The `outb %%al, $0x80` is for synchronization. We don't want to write
 * to the ports so fast that data gets lost.. port 0x80 is an unused port so
 * writing to it is relatively safe. Taken from Linux-2.6
 */	
ICW1 = 0x11
ICW2M = 0x20
ICW2S = 0x28
ICW3M = 0x4
ICW3S = 0x2
ICW4 = 0x1
	
.global init_int_controller


/* Initialize the i8259 interrupt controller. We send the Initialization
 * Control Words in the sequence prescribed by the 8259 datasheet.
 */

init_int_controller:
	pushl %ebp
	movl %esp, %ebp

	pushl %eax
	movb $ICW1, %al
	outb %al, $0x20
	outb %al, $0x80
	outb %al, $0xA0
	outb %al, $0x80	
	
	movb $ICW2M, %al
	outb %al, $0x21
	outb %al, $0x80
	movb $ICW2S, %al
	outb %al, $0xA1
	outb %al, $0x80
	
	movb $ICW3M, %al
	outb %al, $0x21
	outb %al, $0x80	

	movb $ICW3S, %al
	outb %al, $0xA1
	outb %al, $0x80	

	movb $ICW4, %al
	outb %al, $0x21
	outb %al, $0x80
	outb %al, $0xA1
	outb %al, $0x80

	popl %eax
	
	leave
	ret






