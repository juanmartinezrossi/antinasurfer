/*
 * str.c
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

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"

#include "str.h"

/** Max length of the string in str object.
 * This limit is to avoid overwrites
 */

#define T str_o

/** str Object Constructor
 *
 * @return !=0 object pointer
 * @return =0 if error
 * 
 */
T
str_new (void)
{
	T str;

	NEW(str);
	if (str) {
		str->len = 0;
	}
	return str;
}

/** str Object Constructor from string
 *
 * Equivalent to str_new + str_set
 * 
 * @return !=0 object pointer
 * @return =0 if error
 * 
 */
T
str_snew (char *s)
{
	T str;

	NEW(str);
	if (str) {
		if (!str_set (str, s)) {
			str_delete (&str);
		}
	}
	return str;
}

/** str Object Destructor 
 */
void str_delete(T *str)
{
	assert (str);
	assert (*str);
	
	DELETE(*str);
	str = NULL;

}

/** Access str String 
 */
char * str_str(T x)
{
	assert (x);

	return x->str;
}

void str_clear(T x)
{
    assert(x);
    x->len=0;
}

int str_cat(T str, char *src)
{
	int src_len;

	assert (str);
	assert (src);

	if (!*src) {
		return 1;
	}

	src_len=strlen(src)+1+strlen(str->str);
	if (src_len > MAX_STRING_LEN || src_len<=1) {
			printf("CAUTION: str_set did not copied anything\n");
			return 0;
	}

	memcpy(&str->str[str->len], src, strlen(src)+1);

	if (str->len != src_len) {
		str->len = src_len - 1;
	}
	return src_len;
}


/** Modify str Object String
 *
 * Modifies the current string of the str object with a new string.
 * String must be terminated with a null character.
 * In case that the new string length is different than the previous, a reallocation
 * will take place.
 *
 * @param str Object pointer
 * @param src New string to save
 *
 * @return 1 if ok
 * @return 0 if error
 */

int str_set(T str, char *src)
{
	int src_len;

	assert (str);
	assert (src);

	if (!*src) {
		return 1;
	}

	src_len=strlen(src)+1;
	if (src_len > MAX_STRING_LEN || src_len<=1) {
            printf("CAUTION: str_set did not copied anything\n");
            return 0;
	}

	if (str->len != src_len) {
		str->len = src_len - 1;
	}
	strncpy(str->str, src, src_len);
	return src_len;
}

/** Get string value
 *
 * Copies string to user-provided buffer until max_len - 1 characters
 *
 * @param str Object pointer
 * @param dst Pointer where data will be saved
 * @param max_len Size of the buffer in bytes (max_len - 1 characters will be stored).
 *
 * @return 1 if ok
 * @return 0 if error
 */

int str_get(T str, char *dst, int max_len)
{
	int dst_len;

	assert (str && dst && max_len> 0);

	if (!str->str || !str->len) {
		return -1;
	}

	dst_len = str->len;
	if (dst_len + 1> max_len) {
		dst_len = max_len - 1;
	}
	memcpy(dst, str->str, dst_len);
	dst[dst_len] = '\0';

	return dst_len + 1;
}

/** str Compare
 *
 * Compares two strs objects. If sizes are different, it compares 
 * until the lowest one.
 * 
 */
int str_cmp(T x, T y)
{
	int lowest;

	assert (x);
	assert (y);

	lowest = (x->len>y->len) ? y->len : x->len;
	
	return strncmp(x->str, y->str, MAX_STRING_LEN);
}

/** str Compare with string
 *
 * Compares a str object with an string. If sizes are different, it compares 
 * until the lowest one.
 * 
 */
int str_scmp(T y, char *str)
{
	int lowest;
	int len;

	assert (str && y);

	len = strlen(str) + 1;

	return strncmp(str, y->str, MAX_STRING_LEN);
}

/** str duplicate
 *
 * Creates a new str with the same contents 
 */
T
str_dup (T str)
{
	T newstr;

	assert (str);
	if (str->len && str->str) {
		newstr = str_snew(str->str);		
	} else {
		newstr = str_new ();
	}
	return newstr;
}

/** Copy string contents.
 * 
 * Overwrittes dst string with src one.
 */
int str_cpy(T dst, T src)
{
	assert (dst && src);

        strncpy(dst->str,src->str,src->len);
        dst->len=src->len;
        return src->len;
}

int str_len(T str)
{
	assert(str);
	return str->len;
}
int str_topkt(void *x, char **start, char *end)
{
	T s = (T) x;
	int n;

	assert (x && start && end);
	assert (end >= *start);

	if (end - *start < s->len + 1) {
		return 0;
	}

	if (s->len == 0) {
		**start = '\0';
		(*start)++;
		return 1;
	}

	n = str_get((T) x, *start, end - *start);
	if (n < 0) {
		return 0;
	} else {
		*start += n;
		return 1;
	}
}

void * str_newfrompkt(char **start, char *end)
{
	T s;
	assert (start && end);
	assert (end >= *start);

	if (end == *start) {
		return NULL;
	}
	
	s = str_snew((char*) *start);
	*start += s->len + 1;

	return (void*) s;
}

void str_xdelete(const void *x)
{
	str_delete((T*) &x);
}

void * str_xdup(const void *x)
{
	return (void *) str_dup((T) x);
}

int str_xsizeof(const void *x)
{
	return str_len((T) x);
}
