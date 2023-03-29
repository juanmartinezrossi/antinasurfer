/*
 * str.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
 *
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef str_INCLUDED
#define str_INCLUDED

#define MAX_STRING_LEN  128

#define T str_o
#define str_sizeof sizeof(struct str_o)

/** @defgroup str ALOE str
 *
 * @ingroup common_phal
 *
 * Object to manage strings. 
 *
 * @{
 */ 

typedef struct T *T;

struct T
{
	unsigned char len; /* change type if length is going to be bigger */
	char str[MAX_STRING_LEN];
};



T       str_new (void);
T       str_snew (char *s);
void    str_delete (T *str);
void    str_clear(T x);

int 	str_cat(T str, char *src);
int     str_len(T str);
char *  str_str (T str);
int     str_set (T str, char *src);
int     str_get(T str, char *dst, int max_len);

int     str_cmp (T x, T y);
int     str_scmp (T x, char *str);

T       str_dup (T str);
int     str_cpy (T dst, T src);


/* unprotected functions */
void * str_newfrompkt (char **start, char *end);
int     str_topkt (void *x, char **start, char *end);
void str_xdelete (const void *x);
void * str_xdup (const void *x);
int str_xsizeof (const void *x);

/** @} */ 
#undef T
#endif /*  */
