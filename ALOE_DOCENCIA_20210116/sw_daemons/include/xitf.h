/*
 * xitf.h
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
#ifndef xitf_INCLUDED
#define xitf_INCLUDED

#define T xitf_o

#define xitf_sizeof sizeof(struct xitf_o)


/** @defgroup app ALOE Application 
 * @ingroup common_obj
 *
 * Manages an application, its execution status and the objects belonging to it. 
 *
 * @{
 */

typedef struct T *T;

struct T {
    struct hwapi_xitf_i xitf;
    float totalBW;
    xitfid_t remote_id;
    peid_t remote_pe;
};

T xitf_new();
T xitf_dup(T x);
void xitf_delete(T *app);
int xitf_findid(const void *x, const void *member);
int xitf_findremotepe(const void *x, const void *member);
int xitf_topkt(void *x, char **start, char *end);
void * xitf_newfrompkt(char **start, char *end);
void xitf_xdelete(void **x);

/** @} */

#undef T
#endif /*  */
