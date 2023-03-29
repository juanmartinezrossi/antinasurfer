/*
 * pe.c
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

#include "phid.h"
#include "phal_daemons.h"

#include "phal_hw_api.h"

#include "str.h"
#include "set.h"
#include "xitf.h"
#include "pe.h"

#define T pe_o

struct pkt {
	char is_hwman;
	char dummy;
	peid_t id;
	int plat_family;
	int P_idx;
	int BW_idx;
	float C;
	float intBW;
	float totalC;
	float totalintBW;
	int tstamp;
	int nof_cores;
	int tslen_us;
	int nof_objects;
	int exec_order;
	peid_t core_id;
};

#define PE_MIN_PKT_SZ	(1 + sizeof (struct pkt))

T pe_new()
{
    T pe;

    NEW(pe);
    if (!pe) {
        return NULL;
    }
    pe->name = str_new();
    pe->xitfs = Set_new(0, NULL);
    if (!pe->xitfs) {
        pe_delete((T*) & pe);
        return NULL;
    }

    return pe;
}
T pe_dup(T x) {
	T pe;

	NEW(pe);
	if (!pe) {
		return NULL;
	}

    memcpy(pe, x, sizeof (struct pkt));
    pe->name = str_dup(x->name);
	pe->xitfs = Set_dup(x->xitfs, xitf_dup);

	return pe;
}

void pe_xdelete(void ** x)
{
    pe_delete((T*) x);
}

void pe_delete(T *pe)
{
    assert(pe);
    assert(*pe);

    if ((*pe)->xitfs) {
        Set_destroy(&(*pe)->xitfs, xitf_xdelete);
    }

    if ((*pe)->name) {
        str_delete(&(*pe)->name);
    }

    DELETE(*pe);
    pe = NULL;
}

int pe_findid(const void *x, const void *member)
{
    T pe = (T) member;

    return (*((peid_t *) x) != pe->id);
}

int pe_findcoreid(const void *x, const void *member)
{
    T pe = (T) member;

    return (*((peid_t *) x) != pe->core_id);
}

int pe_findPidx(const void *x, const void *member)
{
    T pe = (T) member;

    return (*((int *) x) != pe->P_idx);
}


int pe_topkt(void *x, char **start, char *end)
{
    T pe = (T) x;

    assert(pe && start && end);
    assert(*start <= end);

    if ((end - *start) < PE_MIN_PKT_SZ) {
        return 0;
    }

    memcpy(*start, (void *) pe, sizeof (struct pkt));
    *start += sizeof (struct pkt);

    if (!Set_topkt(pe->xitfs, start, end, xitf_topkt)) {
        return 0;
    }
    if (!str_topkt(pe->name, start, end)) {
        return 0;
    }
    return (*start - end);
}

void * pe_newfrompkt(char **start, char *end)
{
    T pe;

    assert(start && end);
    assert(end >= *start);

    if (end - *start < PE_MIN_PKT_SZ) {
        return NULL;
    }

    NEW(pe);
    if (!pe) {
        return NULL;
    }

    memcpy((void *) pe, *start, sizeof (struct pkt));
    *start += sizeof (struct pkt);

    pe->xitfs = (Set_o) Set_newfrompkt(start, end, xitf_newfrompkt);
    if (!pe->xitfs) {
        pe_delete((T*) & pe);
        return NULL;
    }

    pe->name = str_newfrompkt(start, end);
    if (!pe->name) {
        pe_delete((T*) & pe);
        return NULL;
    }
    return pe;
}
