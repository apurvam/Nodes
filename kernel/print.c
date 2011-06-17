/*********************************************************************
 *                
 * Copyright (C) 2003,  Apurva Mehta (with exceptions, see below)
 *                
 * File path:     kernel/print.c
 * Description:   Implementation of functions to print characters to
 *                the screen.
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


/* The `printf' and `itoa' routines are under the following notice:
 *
 * Copyright (C) 1999  Free Software Foundation, Inc.
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 */


#include <io.h>

extern screen scr;

/* Inserts a newline on the screen */

inline static void insert_newline()
{
	scr.xpos++;
	scr.ypos = 0;
}


static void itoa(char*, int, int);

/* Format a string and print it on the screen, just like the libc
   function printf.  */
void printf (const char *format, ...)
{
	char **arg = (char **) &format;
	int c; 
	char buf[20];

	arg++;
  
	while ((c = *format++) != 0)
	{
		if (c != '%')
			putchar (c);
		else
		{
			char *p;
	  
			c = *format++;
			switch (c)
			{
			case 'd':
			case 'u':
			case 'x':
				itoa (buf, c, *((int *) arg++));
				p = buf;
				goto string;
				break;

			case 's':
				p = *arg++;
				if (! p)
					p = "(null)";

			string:
				while (*p)
					putchar (*p++);
				break;

			default:
				putchar (*((int *) arg++));
				break;
			}
		}
	}
}

/* Put character `c' on the screen at the current cursor position and then
 * move the cursor to the next location.
 */

void putchar (char c)
{
	if ( c == '\n' || c == '\r') insert_newline();
	else if ( c == '\t' ) scr.ypos += 8;
	else{

		if ( scr.ypos >= scr.cols*2 ) insert_newline();

		*(scr.video + (scr.xpos * scr.cols * 2 + scr.ypos++)) = c;
		*(scr.video + (scr.xpos * scr.cols * 2 + scr.ypos++)) = scr.attribute;
		
	}
	
	if ( scr.xpos >= scr.rows) cls();

}


/* Initializes the screen whose properties are in global varible scr
 * of type screen.
 */

void init_screen( unsigned int rows,
		 unsigned int cols, 
		 char attribute)
{
	int i ;
	scr.rows = rows;
	scr.cols = cols;

	scr.attribute = attribute;

	scr.video = (unsigned char*) 0xb8000;

	for ( i = 0; i < rows * cols * 2; i++){
		*(scr.video + i) = 0;
	}

	scr.xpos = 0;
	scr.ypos = 0;
}


/* Clear the screen */
void cls (void)
{
 	int i ; 
	  
	for (i = 0; i < scr.rows * scr.cols * 2; i++){
		*(scr.video + i) = 0;
	} 

	scr.xpos = 0;
	scr.ypos = 0;

}

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal.  */
static void itoa (char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;
  
	/* If %d is specified and D is minus, put `-' in the head.  */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;

	/* Divide UD by DIVISOR until UD == 0.  */
	do
	{
		int remainder = ud % divisor;
      
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while (ud /= divisor);

	/* Terminate BUF.  */
	*p = 0;
  
	/* Reverse BUF.  */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}
