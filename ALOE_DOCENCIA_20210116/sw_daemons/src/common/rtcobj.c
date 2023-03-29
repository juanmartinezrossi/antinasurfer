/*
 * rtcobj.c
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

#include "phid.h"

#include "str.h"
#include "set.h"
#include "rtcobj.h"
#include "phitf.h"
#include "phobj.h"

#include "phal_hw_api.h"

#define T rtcobj_o

T rtcobj_new()
{
    T obj;

    NEW(obj);
    return obj;
}

void rtcobj_delete(T * obj)
{
    assert(*obj);

    DELETE(*obj);
    obj = NULL;
}

T rtcobj_dup(T obj)
{
    T newobj;

    assert(obj);

    NEW(newobj);
    if (newobj) {
        memcpy(newobj, obj, sizeof (struct rtcobj_o));
    }

    return newobj;
}


#define RTCOBJ_MIN_PKT_SZ       (sizeof (struct rtcobj_o))

int rtcobj_topkt(void *x, char **start, char *end)
{
    assert(x && start && end);
    assert(end >= *start);

    if ((end - *start) < RTCOBJ_MIN_PKT_SZ) {
        return 0;
    }

    memcpy(*start, x, sizeof (struct rtcobj_o));
    *start += sizeof (struct rtcobj_o);

    return 1;
}

void * rtcobj_newfrompkt(char **start, char *end)
{
    T obj;

    assert(start && end);
    assert(end >= *start);

    if (end - *start < RTCOBJ_MIN_PKT_SZ) {
        return NULL;
    }

    NEW(obj);
    if (!obj) {
        return NULL;
    }

    memcpy(obj, *start, sizeof (struct rtcobj_o));
    *start += sizeof (struct rtcobj_o);

    return obj;
}

void rtcobj_xdelete(void **x)
{
    rtcobj_delete((T*) x);
}

void * rtcobj_xdup(const void *x)
{
    return (void *) rtcobj_dup((T) x);
}

int rtcobj_xsizeof(const void *x)
{
    return rtcobj_sizeof;
}

