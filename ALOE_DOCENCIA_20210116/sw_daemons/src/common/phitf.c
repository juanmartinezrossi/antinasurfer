/*
 * phitf.c
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
#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"

#include "phid.h"
#include "str.h"
#include "phitf.h"

#define T phitf_o

itfid_t id = 1;

T phitf_new(char mode)
{
    T itf;

    if (mode != FLOW_READ_ONLY && mode != FLOW_WRITE_ONLY
            && mode != FLOW_READ_WRITE) {
        return NULL;
    }

    NEW(itf);
    if (!itf) {
        return NULL;
    }

    itf->mode = mode;
    itf->id = id++;
    itf->name = str_new();
    itf->remotename = str_new();
    itf->remoteobjname = str_new();

    if (!itf->name || !itf->remotename || !itf->remoteobjname) {
        phitf_delete(&itf);
        return NULL;
    }

    return itf;
}

void phitf_delete(T *itf)
{
    assert(itf);
    assert(*itf);



    if ((*itf)->name) {
        str_delete(&(*itf)->name);
    }
    if ((*itf)->remotename) {
        str_delete(&(*itf)->remotename);
    }
    if ((*itf)->remoteobjname) {
        str_delete(&(*itf)->remoteobjname);
    }

    DELETE(*itf);
    itf = NULL;
}

T phitf_dup(T itf)
{
    T newitf;

    assert(itf);

    NEW(newitf);
    if (!newitf) {
        return NULL;
    }

    memcpy(newitf, itf, sizeof (struct topkt));

    newitf->name = str_dup(itf->name);
    newitf->remotename = str_dup(itf->remotename);
    newitf->remoteobjname = str_dup(itf->remoteobjname);

    if (!newitf->name || !newitf->remotename || !newitf->remoteobjname) {
        phitf_delete(&newitf);
        return NULL;
    }

    return newitf;
}

int phitf_isinternal(T x)
{
    T itf = (T) x;

    assert(itf);

    return itf->xitf_id ? 0 : 1;
}

int phitf_iswriteonly(T x)
{
    T itf = (T) x;

    assert(itf);

    return itf->mode == FLOW_WRITE_ONLY;
}

int phitf_isreadonly(T x)
{
    T itf = (T) x;

    assert(itf);

    return itf->mode == FLOW_READ_ONLY;
}

int phitf_isreadwrite(T x)
{
    T itf = (T) x;

    assert(itf);

    return itf->mode == FLOW_READ_WRITE;
}

int phitf_findname(const void *x, const void *member)
{
    T itf = (T) member;

    return str_cmp(itf->name, (str_o) x);
}



#define PHITF_MIN_PKT_SZ        (3 + sizeof (struct topkt))

int phitf_topkt(void *x, char **start, char *end)
{
    T itf = (T) x;

    assert(x && start && end);
    assert(end >= *start);

    if ((end - *start) < PHITF_MIN_PKT_SZ) {
        return 0;
    }

    memcpy(*start, itf, sizeof (struct topkt));
    *start += sizeof (struct topkt);

    if (!str_topkt(itf->name, start, end)) {
        return 0;
    }

    if (!str_topkt(itf->remotename, start, end)) {
        return 0;
    }

    if (!str_topkt(itf->remoteobjname, start, end)) {
        return 0;
    }

    return 1;
}

void * phitf_newfrompkt(char **start, char *end)
{
    T itf;

    assert(start && end);
    assert(end >= *start);

    if ((end - *start) < PHITF_MIN_PKT_SZ) {
        return NULL;
    }

    NEW(itf);
    if (!itf) {
        return NULL;
    }

    memcpy(itf, *start, sizeof (struct topkt));
    *start += sizeof (struct topkt);

    itf->name = str_newfrompkt(start, end);
    if (!itf->name) {
        phitf_delete(&itf);
        return NULL;
    }

    itf->remotename = str_newfrompkt(start, end);
    if (!itf->remotename) {
        phitf_delete(&itf);
        return NULL;
    }

    itf->remoteobjname = str_newfrompkt(start, end);
    if (!itf->remoteobjname) {
        phitf_delete(&itf);
        return NULL;
    }

    return itf;
}

void phitf_xdelete(void **x)
{
    phitf_delete((T*) x);
}

void * phitf_xdup(const void *x)
{
    return (void *) phitf_dup((T) x);
}

