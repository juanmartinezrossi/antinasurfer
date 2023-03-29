/* hwapi_linux_mem.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 * All rights reserved.
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
/**
 * @file phal_hw_api.c
 * @brief ALOE Hardware Library LINUX Implementation
 *
 * This file contains the implementation of the HW Library for
 * linux platforms.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <setjmp.h>

#define __USE_GNU
#include <ucontext.h>
#include <execinfo.h>

#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

#define MAX_MEM_SEGMENTS 40

struct memseg {
    void *ptr;
    int elem_sz;
};
struct memseg memseg[MAX_MEM_SEGMENTS];


extern int mem_silent;

extern int hwapi_initated;






#ifdef TODO
void *hwapi_mem_getbuffer(int size) {

}
#endif




/** @defgroup mem Memory management
 *
 * Functions to use simulated dynamic memory. ALOE does not use dynamic memory
 * for any allocation to prevent system memory fragmentation and detect leaks better.
 *
 * To facilitate programming and introduce the benefits of dynamic memory,
 * ALOE enables the programmer to pre-allocate segments of memory of certain block size.
 * Then, when we request a memory chunk the HW API will search if a segment for this
 * block size has been pre-allocated, returning error or allocating memory to system heap
 * depending on the constant MEM_NOCHUNK_ERROR
 *
 * This will prevent memory fragmentation because only certain fragments are allowed
 * (the ones which have been preallocated).
 *
 *
 * @{
 */

/** Pre-allocate a heap of certain block size
 *
 * @param nof_elems Number of blocks to allocate
 * @param elem_sz Size of the block
 */
int hwapi_mem_prealloc(int nof_blocks, int block_sz)
{
    int i = 0;
    char *buffer;
    int buffer_sz;

    while (i < MAX_MEM_SEGMENTS && memseg[i].ptr)
        i++;
    if (i == MAX_MEM_SEGMENTS) {
        printf("HWAPI: No more memory pre-allocations allowed\n");
        return 0;
    }
    buffer_sz = mem_calc_buffer(nof_blocks, block_sz);
    buffer = calloc(buffer_sz, 1);
    if (!buffer) {
        printf("HWAPI: Error allocating memory (%d of %d)\n", nof_blocks, block_sz);
        return 0;
    }
    memseg[i].ptr = mem_new(buffer, buffer_sz, block_sz);
    if (!memseg[i].ptr) {
        printf("HWAPI: Error creating memory segment\n");
        return 0;
    }
    memseg[i].elem_sz = block_sz;

    return 1;
}


/** dissaloc a pre-allocated memory segment of certain block
 * @param block_sz Size of the block
 */
int hwapi_mem_disalloc(int block_sz)
{
    int i = 0;
    char *x;

    while (i < MAX_MEM_SEGMENTS && memseg[i].elem_sz != block_sz)
        i++;
    if (i == MAX_MEM_SEGMENTS) {
        printf("HWAPI: Didn't find pre-allocated segment of block %d\n", block_sz);
        return 0;
    }
    x = memseg[i].ptr;
    mem_delete(&memseg[i].ptr);
    free(x);
    memset(&memseg[i], 0, sizeof (struct memseg));

    return 1;
}
int memCnt = 0;

void hwapi_mem_init(void) {
    memset(memseg, 0, sizeof (struct memseg) * MAX_MEM_SEGMENTS);
}

void * hwapi_mem_alloc_var(int size)
{

}

/** Allocate a segment of memory
 * @param size Size of the segment to allocate
 */
void * hwapi_mem_alloc(int size)
{
    int i, offset;

    if (hwapi_initated) {
        memCnt++;
        /* find one of equal elem sz */
        for (i = 0; i < MAX_MEM_SEGMENTS; i++) {
            if (memseg[i].elem_sz == size) {
                offset = mem_calloc(memseg[i].ptr, size);
                if (offset > 0) {
                    return (offset + memseg[i].ptr);
                }
            }
        }

    #ifdef MEM_NOCHUNK_ERROR
        return NULL;
    #endif

#ifdef VERBOSE_MEM
        if (!mem_silent) {
            printf("HWAPI: Caution pid %d is allocating memory to system heap (size %d not pre-allocated).\n", getpid(), size);
            print_bt(NULL);
        }
#endif

    }
    return calloc(1, size);
}

/** Free a segment of allocated memory allocated with hwapi_mem_alloc
 *
 * @param ptr Pointer of the segment allocated with hwapi_mem_alloc
 */
void hwapi_mem_free(void *ptr)
{
    int i;

    memCnt--;

    /* find one of equal elem sz */
    for (i = 0; i < MAX_MEM_SEGMENTS; i++) {
        if ((char*) ptr > (char*) memseg[i].ptr && memseg[i].ptr) {
            if (mem_free(memseg[i].ptr, (char*) ptr - (char*) memseg[i].ptr)) {
                break;
            }
        }
    }

    if (i == MAX_MEM_SEGMENTS) {
        #ifdef MEM_NOCHUNK_ERROR
            printf("HWAPI: Caution dellaocatin memory not allocated\n");
            return;
        #endif
#ifdef VERBOSE_MEM
        if (!mem_silent) {
            printf("HWAPI: Caution de-allocating memory from system heap (0x%x)\n", (unsigned int)  ptr);
            print_bt(NULL);
        }
#endif
        free(ptr);
    }
}
/**@} */

/* functions for debugging */
void hwapi_mem_silent(int silent)
{

    mem_silent = silent;
}


int hwapi_mem_used_chunks()
{

    return memCnt;
}
