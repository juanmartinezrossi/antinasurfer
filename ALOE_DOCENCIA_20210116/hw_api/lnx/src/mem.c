/*
 * mem.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "mem.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include "phal_hw_api.h"
#include "hwapi_backd.h"

#define T mem_o

struct chunk {
    int offset;
    short slot_st;
    short slot_end;
};

struct T {
    int nof_slots;
    int slot_sz;
    int buff_sz;
    int slots_offset;
    int chunks_offset;
    int data_offset;
};


int mem_calc_buffer(int nof_slots, int slot_sz)
{
    return sizeof (struct mem_o) + nof_slots * (slot_sz + 1 + sizeof (struct chunk));
}

/** Create new memory pool region.
 *  This function creates a new pool given a memory region
 * defined by an address and a size. Within this buffer, the
 * function will allocate needed data to manage the pool.
 * The user must, also, provide the size of the minimun
 * memory slot that can be assigned.
 * 
 * @param addr Pointer to the buffer
 * @param buff_sz Size (in bytes) of the buffer
 * @param slot_sz Size (in bytes) of the minimum assigned slot.
 * 
 * @return Pointer to the object (same as addr), null if error.
 */
void * mem_new(void *addr, int buff_sz, int slot_sz)
{
    T mem;
    int offset, i;
    int nof_slots;

    assert(addr && slot_sz > 0 && buff_sz > 0);

    memset(addr, 0, buff_sz);

    /** First calculate a number of slots.
     * This number (N) depends on buffer size, slot size
     * and the needed size for the internal management:
     *
     * N = (buff_sz - man_sz) / slot_sz;
     * man_sz = object_sz + 1*N + chunk_sz*N
     * N = (buff_sz - object_sz) / (slot_sz + 1 + chunk_sz)
     *
     * where the lowest integer is taken from the division.
     * This means that at the end some data may be unused.
     */
    nof_slots = (int) (buff_sz - sizeof (struct mem_o)) / (slot_sz + 1
            + sizeof (struct chunk));
    if (nof_slots <= 0) {
        return NULL;
    }

    offset = 0;
    /* set mem object at the beggining */
    mem = (T) addr;
    offset += sizeof (*mem);

    /* next chunks identifiers */
    mem->chunks_offset = offset;
    offset += nof_slots * sizeof (struct chunk);

    /* next used slots identifiers */
    mem->slots_offset = offset;
    offset += nof_slots;

    offset = ((offset-1)>>2)<<2;
    
    mem->data_offset = offset;
    offset += nof_slots * slot_sz;

    /* here if offset > buff_sz a fatal error ocurred */
    assert(offset <= buff_sz);

    /* save parameters */
    mem->nof_slots = nof_slots;
    mem->buff_sz = buff_sz;
    mem->slot_sz = slot_sz;

    struct chunk *chunks;
    chunks = (struct chunk *) (addr + mem->chunks_offset);
    for (i = 0; i < mem->nof_slots; i++) {
        chunks[i].offset = -1;
    }

    return addr;
}

void mem_delete(void *addr)
{
    T mem = (T) addr;

    assert(addr);

    memset(addr, 0, mem->buff_sz);
}

void mem_cpy(T mem, void *dst, void *src, int len)
{
    assert(mem && dst && src && len >= 0);
    memcpy(dst, src, len);
}

int mem_alloc(void *addr, int size)
{
    int i, j, chidx;
    void *p;
    struct chunk *chunks, *chunk;
    char *slots;

    T mem = (T) addr;

    assert(mem && mem && size >= 0);

    p = addr + mem->chunks_offset;
    chunks = (struct chunk *) (addr + mem->chunks_offset);
    slots = (char *) addr + mem->slots_offset;
    
    /* get a free chunk */
    for (i = 0; i < mem->nof_slots; i++) {
        if (chunks[i].offset == -1) {
            break;
        }
    }

    if (i == mem->nof_slots) {
        return -1;
    }

    chunk = &chunks[i];
    chidx = i;

    /* find available slots for required memory */
    j = 0; /* slot start index */
    i = 0; /* slot end index (+1) */

    do {
        if (slots[i]) {
            j = i + 1;
        }
        i++;
    } while (((i - j) * mem->slot_sz < size) && i < mem->nof_slots);

    if ((i - j) * mem->slot_sz < size) {
        #ifdef DEB_MEM
        printf("Not enough slots of size %d pid %d\n", size, getpid());
        #endif
        return -1;
    }

    /* save chunk propierties */
    chunk->offset = mem->data_offset + j * mem->slot_sz;
    chunk->slot_st = j;
    chunk->slot_end = i;

    /* alloc space */
    for (; j < i; j++) {
        slots[j] = 1;
    }

    return chunk->offset;
}

int mem_calloc(void *addr, int size)
{
    int offset;
    T mem = (T) addr;

    assert(mem);

    offset = mem_alloc(mem, size);
    if (offset > -1) {
        memset(addr + offset, 0, size);
    }
    return offset;
}

int mem_free(void *addr, int offset)
{
    int i, chkidx;
    struct chunk *chunks, *chunk;
    char *slots;
    T mem = (T) addr;

    assert(mem);

    /* do nothing if pointer is 0 */
    if (offset < 0) {
        return 0;
    }
    chunks = (struct chunk *) (addr + mem->chunks_offset);
    slots = (char *) (addr + mem->slots_offset);

    /* find chunk */
    for (i = 0; i < mem->nof_slots; i++) {
        if (chunks[i].offset == offset) {
            break;
        }
    }

    if (i == mem->nof_slots) {
        return 0;
    }

    chunk = &chunks[i];
    chkidx = i;

    for (i = chunk->slot_st; i < chunk->slot_end; i++) {
        slots[i] = 0;
    }

    /* and delete chunk data */
    memset(chunk, 0, sizeof (struct chunk));

    chunk->offset = -1;

    return 1;
}
