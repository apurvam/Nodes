/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta
 *                
 * File path:     include/io.h
 * Description:   Contains the declarations for the kernel input,
 *                output functions.
 * 
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

#ifndef __IO_H__
#define __IO_H__

#include <asm/io.h>


/* The screen structure. Maintains the current cursor position, the
 * total rows and columns and a pointer to the video memory.
 * It also holds the attributes (foreground and background colour) for
 * all characters that are displayed on the screen
 */
 

typedef struct screen{
	unsigned int xpos;
	unsigned int ypos;

	unsigned int rows;
	unsigned int cols;

	volatile unsigned char* video;
	char attribute ;
} screen;

/* Forward declarations.  */

/* Clears the screen */

void cls (void);


/* Places the character c into the current cursor postion and moves
 * the cursor to the next postion
 */
void putchar (char c );

/* Format a string and print it on the screen, just like the libc
   function printf.  */

void printf (const char *format, ...);

/* Initializes the screen whose properties are in global varible scr
 * of type screen.
 */

void init_screen ( unsigned int rows, unsigned int cols, char attribute);


#endif /* __IO_H__ */
