/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     kernel/main.c
 * Description:   The primary entry point of the kernel after we have
 *                reached a stable state after boot. We will probe
 *                devices, install interrupt handlers, enable
 *                interrupts and then fork of the initial process.
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

#include <asm/interrupt.h>
#include <io.h>
#include <nodes/devices.h>
#include <multiboot.h>


/* At this point we are in protected mode. We have an IDT with bogus
 * descriptors and a GDT.
 *
 * Note <17/12/2003>: In the future, I want to reach this stage
 * with paging enabled too. Then I want to be able to do the rest at a
 * high level.
 */


/* This is a structure describing the screen we will be using.
 * 
 * Note <17/12/2003>: This is cheap since all we can do at this stage
 * is read from the keyboard and write to the screen! In the future,
 * we must cater for proper terminals.
 */


screen scr;

/* The function that starts the ball rolling. 
 *
 * Called from arch/<arch>/boot/boot.S 
 *
 * A brief explanation of the arguments is in order: 
 *
 * u32_t magic: This is the magic number passed by the multi-boot
 * compliant bootloader
 *
 * u32_t addr: The physical address of the multiboot information
 * structure.
 */


void test_page_alloc (void); /* A temporary function to test the page
			      * allocator */



void kstart() 
{

	init_screen(25, 80, 7); /* Initialize the screen */

	printf ("Welcome to Nodes\n\n");

	printf("Enabling Interrupts..");
	init_interrupts(); /* Setup the interrupt handling system and
			    * enable interrupts.
			    */

	printf("done\n");

	printf("Detected PS/2 Keyboard.\n");
	printf ("Initializing Keyboard..");

	kb_init(); /* Initialize the keyboard. */

	printf("done\n");

	test_page_alloc();

	printf ("\nYou may begin testing the keyboard now.\n");

/* Fork the init process */
/* fork(); */
 
/* Thats it. Goodbye! */

}

