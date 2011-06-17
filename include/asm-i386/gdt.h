/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     include/asm/gdt.h
 * Description:   Routines for setting descriptors in the GDT and also
 *                loading them into the the necessary segment registers.
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

#ifndef __GDT_H__
#define __GDT_H__

#include <sys/types.h>


/* These are the various types of segment descriptors for the
 * IA-32. The encoding is simple:
 * D == data
 * C == code
 * RO == read-only
 * RW == read-write
 * EO == execute-only
 * ER == execute-read
 * A == accessed
 * DOWN == expand-down
 * CONF == conforming
 *
 * So for example, D_RW_DOWN makes the segment a data, read-write,
 * expand-down segment.
 *
 * You can use these macros for the `type' argument of set_seg_desc
 *
 * Oh how simple things have become :)
 */

#define D_RO            0
#define D_RO_A          1
#define D_RW            2
#define D_RW_A          3
#define D_RO_DOWN       4
#define D_RO_A_DOWN     5
#define D_RW_DOWN       6
#define D_RW_A_DOWN     7
#define C_EO            8
#define C_EO_A          9
#define C_ER            10
#define C_ER_A          11
#define C_EO_CONF       12
#define C_EO_A_CONF     13
#define C_ER_CONF       14
#define C_ER_A_CONF     15


typedef struct {
	u32_t dword1, dword2;
} gdt_desc_t;


extern gdt_desc_t __gdt[];


/* BIG FAT NOTE : The interrupt gates that are filled in the idt
 * assume that code segment for the interrupt service routine is at
 * the 3rd slot of the GDT ie.. offset 0x10 (taking into account the
 * mandatory null entry at the beginning
 *
 * So take this into account if you make any changes to the gdt.
 */

static inline void set_seg_desc ( u32_t *gate_addr, u8_t dpl, u8_t type, 
				  u32_t base, u32_t limit)
{
	*gate_addr = ((base & 0xffff) << 16) | (limit & 0xffff);
	gate_addr++;

	*gate_addr = (base & 0xff000000) |  
		(0xC << 20) | 
		(limit & 0x000f0000) | 
		(1 << 15) | 
		(dpl << 13) |
		(1 << 12) | 
		(type << 8) | 
		( (base & 0x00ff0000) >> 16);
}


void init_gdt (void);

#endif /* __GDT_H__ */
