/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     arch/i386/kernel/irq.S
 * Description:   The basic irq handlers that are fed into the IDT. 
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

.global _irq0_hdl, _irq1_hdl, _irq2_hdl, _irq3_hdl, _irq4_hdl, _irq5_hdl 
.global	_irq6_hdl, _irq7_hdl,_irq8_hdl,_irq9_hdl,_irq10_hdl,_irq11_hdl,    
.global	_irq12_hdl,_irq13_hdl,_irq14_hdl,_irq15_hdl



/* Acknowledge the master PIC */
ack_8259_master:
	pushl %ebp
	movl %esp, %ebp

	movb $0x20, %al
	outb %al, $0x20

	leave
	ret

/* Acknowledge the slave PIC */
ack_8259_slave:
	pushl %ebp
	movl %esp, %ebp

	movb $0x20, %al
	outb %al, $0xa0
	outb %al, $0x20

	leave
	ret


/* The following are generic ISRS which do the following 3 things :
 * 1) Save the machine state
 * 2) Call the common handler which cycles through the actual ISRS
 *    for each irq.
 * 3) Restore machine state
 *
 * These are the ISR's that are actually stored in the IDT .  	
 */


	
_irq0_hdl:
	pusha			/* Save state */
	pushl $0		/* Push the irq number */
	call handle_irq	        /* Handle the irq */
	addl $4, %esp		/* Restore the correct esp */
	call ack_8259_master	/* Acknowledge the interrupt */
	popa			/* Restore state */
	iret			/* Return to the interrupted procedure */



_irq1_hdl:
	pusha
	pushl $1
	call handle_irq
	addl $4, %esp
	call ack_8259_master
	popa
	iret


_irq2_hdl:
	pusha
	pushl $2
	call handle_irq
	addl $4, %esp	
	call ack_8259_master
	popa
	iret


_irq3_hdl:
	pusha
	pushl $3
	call handle_irq
	addl $4, %esp
	call ack_8259_master
	popa
	iret


_irq4_hdl:
	pusha
	pushl $4
	call handle_irq
	addl $4, %esp	
	call ack_8259_master
	popa
	iret


_irq5_hdl:
	pusha
	pushl $5
	call handle_irq
	addl $4, %esp
	call ack_8259_master
	popa
	iret


_irq6_hdl:
	pusha
	pushl $6
	call handle_irq
	addl $4, %esp	
	call ack_8259_master
	popa
	iret


_irq7_hdl:
	pusha
	pushl $7
	call handle_irq
	addl $4, %esp	
	call ack_8259_master
	popa
	iret

_irq8_hdl:
	pusha
	pushl $8
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret
		


_irq9_hdl:
	pusha
	pushl $9
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret


_irq10_hdl:
	pusha
	pushl $10
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret

	
_irq11_hdl:
	pusha
	pushl $11
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret

	
_irq12_hdl:
	pusha
	pushl $12
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret

	
_irq13_hdl:
	pusha
	pushl $13
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret


	
_irq14_hdl:
	pusha
	pushl $14
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret


	
_irq15_hdl:
	pusha
	pushl $15
	call handle_irq
	addl $4, %esp
	call ack_8259_slave
	popa
	iret


