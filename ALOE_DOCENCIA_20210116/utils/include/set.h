/**
 * The author of this software is David R. Hanson.
 *
 * Copyright (c) 1994,1995,1996,1997 by David R. Hanson. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose, subject to the provisions described below, without fee is
 * hereby granted, provided that this entire notice is included in all
 * copies of any software that is or includes a copy or modification of
 * this software and in all copies of the supporting documentation for
 * such software.

 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY REPRESENTATION OR
 * WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS SOFTWARE OR
 * ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * David Hanson / drh@microsoft.com / http://www.research.microsoft.com/~drh/
 * $Id: CPYRIGHT,v 1.2 1997/11/04 22:31:40 drh Exp $
 *
 * Modified at 2009 by Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 */

#ifndef SET_INCLUDED
#define SET_INCLUDED
#define T Set_o


struct T
{
	int length;
	unsigned timestamp;
	void * (*hashfield)(const void *x);
	unsigned int (*hash)(const void *x);
	int size;
	struct member
	{
		struct member *link;
		void *member;
	}**buckets;
};

#define setmember_sizeof (sizeof (struct member))

#define set_sizeof (sizeof (struct Set_o) + setmember_sizeof)

/** @defgroup set Set
 * @ingroup common_base
 * @{
 */
typedef struct T *T;

extern T    Set_new (int hint, 	void * hashfield (const void *x));
			/* uncoment this to include the possibility
			* to modify the hash function, by now it is a default function
			unsigned hashfnc (const void *x));
			*/

extern void Set_clear(T set);
extern void Set_delete(T *set);

extern void Set_destroy (T * set, void elem_delete (void **x));

extern int   Set_length(T set);

extern int   Set_member(T set, const void *member);

extern void *Set_get(T set, int i);

extern void *Set_find(T set, const void *data,
			int cmp (const void *x, const void *y));

extern void *Set_find2(T set, const void *data1,
			const void *data2,
			int cmp1 (const void *x, const void *y),
			int cmp2 (const void *x, const void *y));
			

extern void  Set_put   (T set, void *member);

extern void *Set_remove(T set, const void *member);

extern void **Set_toArray(T set, void *end);

extern void  Set_cpy	(T dst, T src, void cpy(const void *dst, const void *src));

extern T Set_dup	(T set, void * elem_dup (const void *x));

extern int Set_topkt	(void *x, char **start, char *end, 
				int elem_topkt (void *x, char **start, char *end));

void * Set_newfrompkt (char **start, char *end, void * elem_newfrompkt (char **start, char *end));

int Set_sizeof		(T set, int elem_sizeof (const void *x));

/*
extern void   Set_map    (T set,
	void apply(const void *member, void *cl), void *cl);

extern T Set_union(T s, T t);

extern T Set_inter(T s, T t);

extern T Set_minus(T s, T t);

extern T Set_diff (T s, T t);
*/

/** @} */

#undef T
#endif
