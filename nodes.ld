/*********************************************************************
 *                
 * Copyright (C) 2004,  Apurva Mehta
 *                
 * File path:     nodes.ld
 * Description:   The linker script
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

OUTPUT_FORMAT (elf32-i386)

__kernel_virt_addr = 0xC0000000;
__kernel_load_addr = 0x100000;

start = (_start - __kernel_virt_addr) + __kernel_load_addr;


SECTIONS
{

	. = __kernel_virt_addr;
		
	__kernel_img_begin = .;
	
	.text : AT (__kernel_load_addr)
	{ 

	__text_begin = .;
	
	*(.text) 
	*(.rodata)

	__text_end = .;
	
	}

	__phys_data =  __kernel_load_addr + ( __data_begin - __text_begin);

	.data : AT ( __phys_data )
	{ 
	
	__data_begin = . ;
	
	*(.data) 
	
	__data_end = . ;
	
	}

	__phys_bss = __kernel_load_addr + ( __bss_begin - __text_begin);

	.bss : AT ( __phys_bss)
	{ 
	__bss_begin = . ;  
	
	*(COMMON) 
	*(.bss) 		
	__bss_end = . ;
		
	} 

 
	. = ALIGN(4096);
	__kernel_img_end = .;


}
