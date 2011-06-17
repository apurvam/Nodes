/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     arch/i386/kernel/interrupt.c
 * Description:   All interrupt related routines for the IA-32.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 ********************************************************************/

#include <sys/types.h>
#include <asm/interrupt.h>
#include <asm/io.h>
#include <io.h>


/* Forward declarations ofthe generic irq handlers defined in irq.S */
void _irq0_hdl (void);
void _irq1_hdl (void);
void _irq2_hdl (void);
void _irq3_hdl (void);
void _irq4_hdl (void);
void _irq5_hdl (void);
void _irq6_hdl (void);
void _irq7_hdl (void);
void _irq8_hdl (void);
void _irq9_hdl (void);
void _irq10_hdl (void);
void _irq11_hdl (void);
void _irq12_hdl (void);
void _irq13_hdl (void);
void _irq14_hdl (void);
void _irq15_hdl (void);


/* A table for the 16 irq's */
static irq_t irq_table[NUM_OF_IRQS]; 


/* Set a handler for a particular irq number */

void set_irq_handler (u32_t irq_num, int_handler_t handler)
{

	irq_t *irq;

	if (irq_num >= NUM_OF_IRQS){
		printf ("ERROR: Invalid irq number : %d\n", irq_num);
		return;
	}


	irq = &irq_table[irq_num];

	if ( irq->num_of_isrs >= MAX_SHARED_INTERRUPTS ){
		printf ("Error: Too many devices sharing irq number : %d\n", irq->num);
	}
	else{
		irq->isr[irq->num_of_isrs++] = handler;
	}

}

/* This fuction is called from the _irqN_hdl functions which are
 * stored in the idt (see irq.S). It retrieves the irq_t structure for the
 * particular irq to be handled and then uses the information to call
 * each registered ISR in turn.
 */

void handle_irq (u32_t irq_num)
{
	irq_t *irq = &irq_table[irq_num];

       	if (irq->num_of_isrs == 0) printf ("Error: No ISR's registered for irq %d\n", irq_num);
	else{
		int i;
		int_handler_t isr;
		for (i = 0; i < irq->num_of_isrs ; i++){
			isr = irq->isr[i];
			isr();
		}
	}
}



/* Stores the passed handler `handler' into the IDT at location
 * corresponding to `vector'
 * 
 * BIG FAT NOTE : The interrupt gates that are filled in the idt
 * assume that code segment for the interrupt service routine is at
 * the 3rd slot of the GDT ie.. offset 0x10 (taking into account the
 * mandatory null entry at the beginning)
 *
 * So take this into account if you make any changes to the gdt.
 */


static void 
set_intr_gate (u16_t vector, int_handler_t handler)
{								
	__asm__ __volatile__ ("movl %%ebx, %%eax\n\t"	
			      "movw $0x8e00, %%ax\n\t"		
			      "movl $0x00100000, %%ecx\n\t"	
			      "movw %%bx, %%cx\n\t"		
			      "movl %%ecx, (%%edx)\n\t"		
			      "movl %%eax, 4(%%edx)"		
			      :: "d" (&__idt[vector]), "b"(handler)	
			      : "%eax", "%ecx"				
		);

}


/* This starts up the interrupt handling system. It should be called
 * only once at system startup. It performs the following functions:
 * 
 * 1) Initializes the PIC,
 * 2) Sets up the IVT with our _irqN_hdl* functions, 
 * 3) Cycles through the irq_table and initializes the
 *    information structure associated with each irq line.
 * 4) Enables interrupts.
 */

void init_interrupts(void)
{
	u32_t i; 
	irq_t *irq;

	init_int_controller();

	set_intr_gate (0x20, _irq0_hdl);
	set_intr_gate (0x21, _irq1_hdl);
	set_intr_gate (0x22, _irq2_hdl);
	set_intr_gate (0x23, _irq3_hdl);
	set_intr_gate (0x24, _irq4_hdl);
	set_intr_gate (0x25, _irq5_hdl);
	set_intr_gate (0x26, _irq6_hdl);
	set_intr_gate (0x27, _irq7_hdl);
	set_intr_gate (0x28, _irq8_hdl);
	set_intr_gate (0x29, _irq9_hdl);
	set_intr_gate (0x2A, _irq10_hdl);
	set_intr_gate (0x2B, _irq11_hdl);
	set_intr_gate (0x2C, _irq12_hdl);
	set_intr_gate (0x2D, _irq13_hdl);
	set_intr_gate (0x2E, _irq14_hdl);
	set_intr_gate (0x2F, _irq15_hdl);


	for (i = 0; i < NUM_OF_IRQS; i++){
		irq = &irq_table[i];
		irq->num_of_isrs = 0;
		irq->num = i;

		disable_irq(i);
	}

	sti();

}


/* Enable the passed irq number. */

void enable_irq (u32_t irq)
{
	if ( irq < 8) {
		outb ( inb(INT_CTLMASK) & ~(1 << irq), INT_CTLMASK);
	}
	else{
		irq -= 8;
		outb ( inb(INT2_CTLMASK) & ~(1 << irq), INT2_CTLMASK);
	}
}


/* Disable the passed irq number */

void disable_irq (u32_t irq)
{
	if (irq < 8 ) outb( inb(INT_CTLMASK) | ( 1 << irq), INT_CTLMASK);
	else{
		irq -= 8;
		outb ( inb(INT2_CTLMASK) | ( 1 << irq), INT2_CTLMASK);
	}
}
