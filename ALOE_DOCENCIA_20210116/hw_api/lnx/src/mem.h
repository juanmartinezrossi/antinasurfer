/*
 * mem.h
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

#ifndef MEM_INCLUDED
#define MEM_INCLUDED
#define T mem_o

/** @defgroup mem Memory Pool manager
 * @ingroup common_base
 * @{
 */
typedef struct T *T;


void * mem_new (void *addr, int buff_sz, int nof_chunks);

void mem_delete (void *addr);

int mem_alloc (void *addr, int size);

int mem_calloc (void *addr, int size);

int mem_free (void *addr, int offset);

void mem_cpy(T mem, void *dst, void *src, int len);

int mem_calc_buffer(int nof_slots, int slot_sz);




/** @} */

#undef T
#endif
