/*
 * xitf.c
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

#define T xitf_o

T xitf_new()
{
    T xitf;

    NEW(xitf);
    if (!xitf) {
        return NULL;
    }

    return xitf;
}

T xitf_dup(T x)
{
    T xitf;

    NEW(xitf);
    if (!xitf) {
        return NULL;
    }
    memcpy(xitf, x, sizeof (struct xitf_o));
    return xitf;
}
void xitf_delete(T *xitf)
{
    assert(xitf);
    assert(*xitf);

    DELETE(*xitf);
    xitf = NULL;
}

int xitf_findid(const void *x, const void *member)
{
    T xitf = (T) member;

    return (*((xitfid_t *) x) != xitf->xitf.id);
}

int xitf_findremotepe(const void *x, const void *member)
{
    T xitf = (T) member;

    return (*((xitfid_t *) x) != xitf->remote_pe);
}

#define XITF_MIN_PKT_SZ	(sizeof (struct xitf_o))

int xitf_topkt(void *x, char **start, char *end)
{
    T xitf = (T) x;

    assert(xitf && start && end);
    assert(*start <= end);

    if ((end - *start) < XITF_MIN_PKT_SZ) {
        return 0;
    }

    memcpy(*start, (void *) xitf, sizeof (struct xitf_o));
    *start += sizeof (struct xitf_o);
    return 1;
}

void * xitf_newfrompkt(char **start, char *end)
{
    T xitf;

    assert(start && end);
    assert(end >= *start);

    if (end - *start < XITF_MIN_PKT_SZ) {
        return NULL;
    }

    NEW(xitf);
    if (!xitf) {
        return NULL;
    }

    memcpy((void *) xitf, *start, sizeof (struct xitf_o));
    *start += sizeof (struct xitf_o);

    return xitf;
}

void xitf_xdelete(void **x)
{
    xitf_delete((T*) x);
}
