/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     include/asm-i386/interrupt.h
 * Description:   Declares fucntions need for interrupt handling
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

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


#include <sys/types.h>

#define MAX_SHARED_INTERRUPTS 10 /* Max number of devices sharing an
				  * irq line 
				  */

#define NUM_OF_IRQS 16           /* Number of IRQ lines 
				  */

#define INT_CTLMASK 0x21         /* Port to write to for masking
				  * interrupts on the master PIC 
				  */ 

#define INT2_CTLMASK 0xA1        /* Port to write to for masking
				  * interrupts on the slave PIC
				  */


/* Clear interrupt flag*/

#define cli()	__asm__ __volatile__ ("cli");


/* Set interrupt flag */


#define sti()	__asm__ __volatile__ ("sti");


/* The type that represents our ISR's */

typedef void (*int_handler_t) (void); 


/* Initialize the interrupt controller. In this case it is IC8259 */

void init_int_controller(void);


/* A structure representing an interrupt descriptor */

typedef struct _int_desc{
	u32_t dword1, dword2;
} _int_desc[256];



/* The actual idt.
 * Space for the IDT is allocated in arch/<arch>/boot/boot.S
 */

extern _int_desc __idt;


/* The irq type. Each irq line has one object of this type. */

typedef struct irq_t{
	u32_t num_of_isrs; /* Number of registered ISR's for this
				    irq */

	u32_t num; /* The irq number */
	int_handler_t isr[MAX_SHARED_INTERRUPTS]; /* Pointers to the
						     * ISRS  */
						    
} irq_t;



/* Set a handler for a particular irq number */
void set_irq_handler (u32_t irq_num,  int_handler_t handler);

/* Enable irq line */
void enable_irq (u32_t irq);


/* Disable irq line */
void disable_irq (u32_t irq);

/* Initialize the interrupt handling system */
void init_interrupts (void); 

#endif /* __INTERRUPT_H__ */
