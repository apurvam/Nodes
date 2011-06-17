/*********************************************************************
 *                
 * Copyright (C) 2004,  Apurva Mehta
 *                
 * File path:     mm/mm.h
 * Description:   Memory management routines and constants that are
 *                required by the VM implementation.
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

#ifndef __VM_MM_H__
#define __VM_MM_H__


#include <sys/types.h>


#undef DEBUG   /* Set this if you want the MMU to print debug
		* information */



#define LOW_MEM_ZONE 0   /* The lower memory zone */

#define HIGH_MEM_ZONE 1  /* The higher memory zone */

#define LOW_MEM_BOUNDARY 0x1000000 /* The boundary of the lower memory
				    * zone */

#define PAGE_SIZE_BYTES 4096 /* The page size in bytes */


#define PAGE_OFFSET 0xC0000000     /* Load the kernel at this address
				    * in the virtual memory region.
				    */


extern u32_t __kernel_img_begin[];  /* A linker symbol which is the
				     * first physical address of the
				     * kernel. It is a virtual
				     * address. */

extern u32_t __kernel_img_end[];    /* A linker symbol which is the 
				     * the first usable address after
				     * the end of the kernel. It is
				     * page aligned. It is a virtual
				     * address */

extern u32_t __kernel_virt_addr[];  /* The start of the kernel in the
				     * virtual address space. */
 
extern u32_t __kernel_load_addr[];  /* The actual physical address of
				     * the start of the kernel */




/* Aligns the address `addr' to the byte boundary specified by
 * `boundary'. Returns the aligned address.
 */

static inline u32_t align_to_boundary (u32_t addr, u32_t boundary)
{
	return addr + (addr % boundary ? boundary - (addr % boundary): 0 );
}


/* Calculates the physical address of a kernel virtual address. Will
 * not work for addresses of objects outside the kernel.
 */

static inline u32_t phys_addr (u32_t addr)
{
	if ( addr >= (u32_t) __kernel_virt_addr){
		return (addr - (u32_t) __kernel_virt_addr + (u32_t) __kernel_load_addr);
	}
	else return addr;
}



/* Initializes the page allocator */

void init_page_alloc ( u32_t upper_mem_kb, u32_t img_phys_end_addr);


/* Allocates a free physical page from the specified zone and returns
 * the address */

u32_t allocate_page (u32_t zone);


/* Deallocates the page whose address is passed. */

void deallocate_page (u32_t addr);


/* This function returns the page aligned physical end address of the
 * kernel which also accounts for the space taken by the page
 * allocator. The page allocator is given space right after the end of
 * the kernel image. */

static inline u32_t kernel_phys_end_addr ( u32_t upper_mem_kb, u32_t img_phys_end_addr)
{

 	u32_t kernel_end_page = img_phys_end_addr / PAGE_SIZE_BYTES;

	u32_t last_page_addr = align_to_boundary ( upper_mem_kb * 1024, PAGE_SIZE_BYTES) \
		- PAGE_SIZE_BYTES;

	u32_t last_page_num =  last_page_addr / PAGE_SIZE_BYTES;

	u32_t num_of_pages = last_page_num - kernel_end_page  + 1;

	u32_t start_usable_mem = img_phys_end_addr + (sizeof (u32_t) * num_of_pages);


#ifdef DEBUG	
	printf ("kernel_end_addr : 0x%x\n", (u32_t)__kernel_img_end);
	printf ("Size take by page allocator : %u bytes\n", start_usable_mem - (u32_t)__kernel_img_end);
	printf ("kernel_end_page : %u\n", kernel_end_page);
	printf ("last_page_addr : 0x%x\n", last_page_addr);
	printf ("last_page_num : %u\n", last_page_num);

	printf ("start_usable_mem: 0x%x\n", start_usable_mem);
	printf ("num_of_pages : %u\n", num_of_pages);

#endif /* DEBUG */

	return align_to_boundary (start_usable_mem, PAGE_SIZE_BYTES);
}

#endif /* __VM_MM_H__  */
