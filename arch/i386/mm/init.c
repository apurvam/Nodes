/*********************************************************************
 *                
 * Copyright (C) 2004,  Apurva Mehta
 *                
 * File path:     mm/init.c
 * Description:   Initialize the memory manager.
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


#include <mm/mm.h>
#include <sys/types.h>
#include <asm/mm.h>
#include <io.h>
#include <multiboot.h>


extern u32_t kernel_pg_dir[];  /* The page tables and page directories
			       * required before we can enable paging */
extern u32_t pg_table1[];
extern u32_t pg_table2[];

multiboot_info_t *_mbi;  /* A global pointer to the multiboot
			  * information structure */


/* =============== pg_dir_index =============== */
/* Returns the page directory index of the passed address */

static inline u32_t pg_dir_index ( u32_t addr )
{
	return (addr & 0xffc00000) >> 22;
}

/* =============== pg_table_index =============== */
/* Returns the page table index of the passed address */

static inline u32_t pg_table_index ( u32_t addr )
{
	return ( addr & 0x3ff000 ) >> 12;
}

/* =============== page_offset =============== */
/* Returns the page offset of the passed address */

static inline u32_t page_offset ( u32_t virt_addr )
{
	return ( virt_addr & 0xfff );
}


/* =============== virt_to_phys =============== */
/* NOTE <14/2/2004> <apurva@gmx.net> : This function needs updating.
 * Returns the physical address corresponding to the passed virtual
 * address. The calculation is done according to the current page
 * table
 */

static inline u32_t virt_to_phys (u32_t virt_addr, u32_t *pg_dir)
{
	u32_t *pg_table = (u32_t *) pg_dir [pg_dir_index (virt_addr)];
	u32_t page = pg_table [ pg_table_index ( virt_addr) ];
	
	return (page + page_offset ( virt_addr));

}


/* =============== insert_pg_dir_entry =============== */
/* For the passed virtual address `virt_addr', this function will
 * insert the page table address `pg_table_addr' into the page
 * directory `pg_dir' at the appropriate location. The flags are set
 * as per the `flags' argument.
 */

static inline void insert_pg_dir_entry ( u32_t virt_addr,
					 u32_t *pg_dir, 
					 u32_t pg_table_addr,
					 u32_t flags )
{
	pg_dir [ pg_dir_index (virt_addr) ] = (pg_table_addr | flags);
}



/* =============== insert_pg_table_entry =============== */
/* For the passed virtual address `virt_addr', this function will
 * insert the page address `phys_page' into the page table
 * `pg_table'. The flags are set as per the `flags' argument.
 */

static inline void insert_pg_table_entry ( u32_t virt_page, 
					   u32_t *pg_table, 
					   u32_t phys_page,
					   u32_t flags)
{
	pg_table [ pg_table_index (virt_page) ] = ( phys_page | flags);
}



/* =============== init_paging =============== */
/* Initializes the paging system. It takes the total installe physical
 * address as well as the physical end address of the kernel image as
 * arguments. This is mainly to determine where the actual end of the
 * kernel after space is allocated for the page allocator.
 *
 * The function does the following in order:
 * 1) It identity maps the video memory.
 * 2) It identity maps the kernel onto itself.
 * 3) It creates mappings from virtual address space of the kernel to
 * its corresponding physical addresses.
 * 4) It identity maps the page directory onto itself. This is needed
 * for the mmap and mumap functions.
 * 5) It sets the page directory on the processor.
 * 6) It enables paging.
 */

void init_paging (u32_t upper_mem_kb, u32_t img_phys_end_addr)
{
	u32_t tmp1 = 0xA000;
	u32_t tmp2 = PAGE_OFFSET;

	u32_t phys_end_of_kernel = kernel_phys_end_addr ( upper_mem_kb, img_phys_end_addr);


	/* Setup the page directory entry for the region 0-4 MB  */
	insert_pg_dir_entry ( 0, 
			      (u32_t *) phys_addr ( (u32_t) kernel_pg_dir), 
			      phys_addr ( (u32_t) pg_table2),
			      PRESENT | RW | ACCESSED);

	/* Identity map the video memory */
	while ( tmp1 <= 0xFF000){
		insert_pg_table_entry ( tmp1, 
					(u32_t *) phys_addr ( (u32_t) pg_table2), 
					tmp1, 
					PRESENT | RW | CACHE_DISABLE | ACCESSED);

		tmp1 += PAGE_SIZE_BYTES;
	}

	tmp1 = (u32_t) __kernel_load_addr;

	/* Identity map the kernel */
	while ( tmp1 < phys_end_of_kernel){
		insert_pg_table_entry ( tmp1, 
					(u32_t *) phys_addr ( (u32_t) pg_table2), 
					tmp1, 
					PRESENT | RW | GLOBAL | ACCESSED);

		tmp1 += PAGE_SIZE_BYTES;
	}


	/* Set the page directory entry for the 4 MB starting at
	 * virtual address 0xC0000000 */
	insert_pg_dir_entry (PAGE_OFFSET, 
			     (u32_t *) phys_addr ( (u32_t) kernel_pg_dir), 
			     phys_addr ( (u32_t) pg_table1), 
			     PRESENT | RW | GLOBAL | ACCESSED);


	tmp1 = (u32_t) __kernel_load_addr;
	tmp2 = PAGE_OFFSET;
	/* Map the virtual kernel address space onto the physical
	 * pages */
	while ( tmp1 < phys_end_of_kernel){
		insert_pg_table_entry ( tmp2, 
					(u32_t *) phys_addr ( (u32_t) pg_table1),
					tmp1, 
					PRESENT | RW | GLOBAL | ACCESSED);

		tmp1 += PAGE_SIZE_BYTES;
		tmp2 += PAGE_SIZE_BYTES;
	}


	/* Map the page directory onto itself in the upper 4 MB of the
	 * virual address space. This is needed for mapping physical
	 * pages into the virtual address space */
	kernel_pg_dir [1023] = (u32_t) kernel_pg_dir | PRESENT | RW | ACCESSED;

	/* Let the CPU know where the page directory is */
	set_pg_dir ( phys_addr ( (u32_t) kernel_pg_dir));

	/* Lets get the show on the road :) */
	enable_paging();

}


/* =============== init_mm =============== */
/* Initialize the virtual memory system. Basically this function gets
 * the total installed physical memory from the bootloader and then
 * calls appropriate functions for initializing paging and for
 * initializing the page allocator.
 */

void init_mm (u32_t magic, u32_t addr)
{
	multiboot_info_t *mbi = (multiboot_info_t *) addr;

	u32_t up_mem_kb = 0;
	
	if ( is_bit_set (mbi->flags, 0) ) up_mem_kb = mbi->mem_upper;

	init_paging (up_mem_kb, phys_addr ( (u32_t) __kernel_img_end) );

	_mbi = mbi;

	init_page_alloc (up_mem_kb, phys_addr ( (u32_t) __kernel_img_end));
}
