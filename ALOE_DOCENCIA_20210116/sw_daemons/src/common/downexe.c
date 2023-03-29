/*
 * downexe.c
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
#include "swman.h"

#include "str.h"
#include "set.h"
#include "rtcobj.h"
#include "phitf.h"
#include "phobj.h"

#include "downexe.h"

#define T downexe_o

int exeid=0;

T downexe_new()
{
    T exe;

    NEW(exe);
    if (!exe) {
        return NULL;
    }
    exe->name = str_new();
    exe->objects = Set_new(0, NULL);
    exe->id = exeid++;
    if (!exe->name || !exe->objects) {
        downexe_delete(&exe);
        return NULL;
    }
    return exe;
}

void downexe_delete(T * exe)
{
    assert(exe);
    assert(*exe);

    if ((*exe)->name) {
        str_delete(&(*exe)->name);
    }
    if ((*exe)->objects) {
        Set_destroy(&(*exe)->objects, phobj_xdelete);
    }

    DELETE(*exe);
    exe = NULL;
}

int downexe_findname(const void *x, const void *member)
{
    T exe = (T) member;

    assert(x && member);

    return str_cmp(exe->name, (str_o) x);

}


int downexe_findid(const void *x, const void *member)
{
    T exe = (T) member;

    assert(x && member);

    return exe->id != *((int*) x);

}


