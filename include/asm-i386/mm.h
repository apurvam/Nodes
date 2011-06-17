/*********************************************************************
 *                
 * Copyright (C) 2004,  Apurva Mehta
 *                
 * File path:     mm/init.c
 * Description:   Architecture specific memory management routines.
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

#ifndef __ASM_MM_H__
#define __ASM_MM_H__

#include <sys/types.h>

/* Flags for PDE's / PTE's */
#define PRESENT            ( (u32_t) 1)
#define RW                 ( (u32_t) 1 << 1)
#define USER_PRIVILEGE     ( (u32_t) 1 << 2)
#define PAGE_WRITE_THROUGH ( (u32_t) 1 << 3)
#define CACHE_DISABLE      ( (u32_t) 1 << 4)
#define ACCESSED           ( (u32_t) 1 << 5)
#define GLOBAL             ( (u32_t) 1 << 8) 

/* #define DIRTY              ( (u32_t) 1 << 6) */
/* #define FOUR_MB_PAGE       ( (u32_t) 1 << 7) */


/* Returns the phyiscal address of the current page directory. */
static inline u32_t *get_curr_pg_dir()
{
	u32_t *pg_dir;

	asm volatile ("movl %%cr3, %0"
			      : "=r" (pg_dir) );

	return pg_dir;
}


/* Stores the passed address into the cr3 register */
static inline void set_pg_dir(u32_t pg_dir)
{
	asm volatile ("movl %0, %%cr3\n\t"
		      "movl %%cr4, %%eax\n\t"
		      "andl $0xffffffcf, %%eax\n\t"
		      "movl %%eax, %%cr4\n\t"
		      :: "r" (pg_dir) : "%eax" );
}

/* Sets bit 31 on the cr0 register to enable paging */
#define enable_paging() asm volatile ("movl %%cr0, %%eax\n\t"		\
				      "orl $0x80000000, %%eax\n\t"	\
				      "movl %%eax , %%cr0"::: "%eax" );

#endif /* __ASM_MM_H__ */
