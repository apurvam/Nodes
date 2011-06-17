/*********************************************************************
 *                
 * Copyright (C) 2004,  Apurva Mehta
 *                
 * File path:     mm/page_alloc.c
 * Description:   A physical page allocator.
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

/* The focus of our physical page allocator is to keep it simple. We
 * use a stack that holds the address of all the free physical pages
 * in the system. Whenever we want a new page, we just pop off the top
 * of the stack. Whenever a page gets freed, we push it back onto the
 * stack.
 *
 * We will be using 2 stacks. One for the lower 16MB of memory, and
 * one for the rest. This is because things like ISA DMA etc require
 * the memory region to be in the lower 16 MB only. By default, we
 *
 * will use the higher 16MB of memory, but if we run out of that, then
 * we will take from the lower 16 MB as well.
 *
 * The only major problem with this scheme is that it is problematic
 * to allocate continuous pages. To the best of my knowledge, this is
 * required only for DMA and hence if we avoid using the lower 16 MB,
 * then when a continuos allocation request comes, we can use that
 * stack directly.
 */


#include <mm/mm.h>

#include <sys/types.h>

#include <io.h>  /* Included mainly for debug purposes */




static u32_t *low_mem;   /* The stack top of all free pages in the
			 * first 16 MB of memory */

static u32_t *high_mem;  /* The stack top of all free pages from the
			 * first 16 MB to the end of memory */

static u32_t *end_of_low_mem;  /* This is a pointer to one past the
				* end of the lower memory stack. */

static u32_t *end_of_high_mem;  /* This is a pointer to one past the
				 * end of the higher memory stack */





/* ================= init_page_alloc ================== */

/* This function initializes the page allocator. It sets up the memory
 * of the page allocator itself and then proceeds to fill the stacks
 * of the lower memory zone and the higher memory zone. 
 */

void init_page_alloc (u32_t upper_mem_kb, u32_t img_phys_end_addr)
{

	u32_t curr_page_addr = kernel_phys_end_addr ( upper_mem_kb,
						      img_phys_end_addr);


/* This the address of the last usable page in the system */

	u32_t last_page_addr = align_to_boundary ( upper_mem_kb * 1024, PAGE_SIZE_BYTES) \
		- PAGE_SIZE_BYTES;

	low_mem = end_of_low_mem = (u32_t *) img_phys_end_addr;


	
#ifdef DEBUG
	printf ("curr_page_addr : 0x%x\n", curr_page_addr);
	printf ("img_phys_end_addr : 0x%x\n", img_phys_end_addr); 
	printf ("low_mem : 0x%x\n", (u32_t)low_mem);
#endif /* DEBUG */


	while ( curr_page_addr < LOW_MEM_BOUNDARY){
		*end_of_low_mem = curr_page_addr;
		end_of_low_mem++;
		curr_page_addr += PAGE_SIZE_BYTES;
       	}


	high_mem = end_of_high_mem = end_of_low_mem;


#ifdef DEBUG
	printf("end_of_low_mem : 0x%x\n",  curr_page_addr);
#endif /* DEBUG */

	
	while (curr_page_addr < last_page_addr){
		*end_of_high_mem = curr_page_addr;
		end_of_high_mem++;
		curr_page_addr += PAGE_SIZE_BYTES;
	}

}


/* ================== allocate_page ================== */

/* The strategy here is simple. If the request is an allocation to the
 * higher memory zone, then check if you have space. If you do then
 * return a page from the higher memory. If you dont, then try to
 * return a page from lower memory. If that fails then you are out of
 * physical memory, try to swap something out to disk.
 *
 * If you try to allocate a page from the lower memory and you fail,
 * then it will scream out that you are out of lower memory. It will
 * not invoke the pager.
 *
 * You must be wondering, how do we allocate pages? Well all we do is
 * pop of the address the top of the relevant stack. Thats it!
 */

u32_t allocate_page (u32_t zone)
{
	u32_t page = 0;

	if ( zone == HIGH_MEM_ZONE && high_mem < end_of_high_mem){
		page = *high_mem;
		high_mem++;
	}
	else if (low_mem < end_of_low_mem){
		page = *low_mem;
		low_mem++;
	}
	else if ( zone == LOW_MEM_ZONE) printf ("\n\nNo more pages in lower memory\n\n");
	else printf ("\n\nOut of physical memory..\n\n");

	
	return page;
}


/* ================== deallocate_page ================== */

/* If you read the comment for allocate_page, then this is pretty self
 * evident. Find the zone to which the `to be freed' page belongs and
 * then push the passed address onto that stack. As simple as A B C
 */

void deallocate_page (u32_t page_addr)
{
	if ( page_addr < LOW_MEM_BOUNDARY){
		low_mem--;
		*low_mem = page_addr;
	}
	else{
		high_mem--;
		*high_mem = page_addr;
	}
}



/* =============== test_page_alloc =============== */

/* Test the page allocator. */


void test_page_alloc (void)
{
	int i;
	u32_t addresses[50];

 	printf ("Allocating 10 pages from upper memory : \n");
	for (i = 0; i < 10; i++) addresses[i] = allocate_page(1);

	printf ("The pages allocated are : \n");
	for ( i = 0; i < 10; i++) printf ("0x%x\t", addresses[i]);

	printf ("\nDeallocating 5  pages from upper memory : \n");
	for (i = 5; i < 10; i++) deallocate_page(addresses[i]);

	printf ("Allocating 10 pages from upper memory : \n");
	for (i = 5; i < 15; i++) addresses[i] = allocate_page(1);

	printf ("The pages allocated are : \n");
	for (i = 5; i < 15; i++) printf ("0x%x\t", addresses[i]);

	printf ("\nDeallocating 15 pages from upper memory : \n");
	for (i = 0; i < 15; i++) deallocate_page (addresses[i]);

	printf ("The first 15 pages in upper memory are : \n");
	for (i = 0; i < 15; i++) printf("0x%x\t", high_mem[i]);

	putchar ('\n');
	

}
