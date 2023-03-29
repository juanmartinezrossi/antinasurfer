/*
 * phobj.c
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
#include "var.h"
#include "set.h"
#include "rtcobj.h"
#include "phitf.h"
#include "phobj.h"

#define T phobj_o

objid_t obj_id_cnt = 0;

T phobj_new(void)
{
    T obj;

    NEW(obj);
    if (!obj) {
        return NULL;
    }

    if (!obj_id_cnt)
        obj_id_cnt++;

    obj->obj_id = obj_id_cnt++;
    obj->itfs = Set_new(0, NULL);
    obj->stats = Set_new(0, NULL);
    obj->params = Set_new(0, NULL);
    obj->appname = str_new();
    obj->objname = str_new();
    obj->exename = str_new();
    obj->rtc = rtcobj_new();
    obj->logs = Set_new(0, NULL);
    obj->force_pe = -1;

    if (!obj->itfs || !obj->stats || !obj->params
            || !obj->appname || !obj->objname || !obj->exename
            || !obj->rtc) {
        phobj_delete(&obj);
        return NULL;
    }

    return obj;
}

void phobj_delete(T *obj)
{
    assert(obj);
    assert(*obj);

    if ((*obj)->itfs) {
        Set_destroy(&(*obj)->itfs, phitf_xdelete);
    }

    if ((*obj)->stats) {
        Set_destroy(&(*obj)->stats, var_xdelete);
    }

    if ((*obj)->params) {
        Set_destroy(&(*obj)->params, var_xdelete);
    }

    if ((*obj)->logs) {
        Set_destroy(&(*obj)->logs, log_xdelete);
    }

    if ((*obj)->rtc) {
        rtcobj_delete(&(*obj)->rtc);
    }

    if ((*obj)->appname) {
        str_delete(&(*obj)->appname);
    }

    if ((*obj)->objname) {
        str_delete(&(*obj)->objname);
    }

    if ((*obj)->exename) {
        str_delete(&(*obj)->exename);
    }

    DELETE(*obj);
    obj = NULL;
}

T phobj_dup(T obj)
{

    T newobj;

    assert(obj);

    NEW(newobj);
    if (!newobj) {
        return NULL;
    }

    if (obj->itfs) {
        newobj->itfs = Set_dup(obj->itfs, phitf_xdup);
    }

    if (obj->stats) {
        newobj->stats = Set_dup(obj->stats, var_xdup);
    }

    if (obj->params) {
        newobj->params = Set_dup(obj->params, var_xdup);
    }

    if (obj->rtc) {
        newobj->rtc = rtcobj_dup(obj->rtc);
    }

    if (obj->appname) {
        newobj->appname = str_dup(obj->appname);
    }

    if (obj->objname) {
        newobj->objname = str_dup(obj->objname);
    }

    if (obj->exename) {
        newobj->exename = str_dup(obj->exename);
    }

    newobj->app_id = obj->app_id;
    newobj->obj_id = obj->obj_id;
    newobj->pe_id = obj->pe_id;
    newobj->status = obj->status;
    newobj->exec_position = obj->exec_position;
    newobj->core_idx = obj->core_idx;
    newobj->force_pe = obj->force_pe;

    return newobj;
}

struct pkt {
    peid_t pe_id;
    objid_t obj_id;    
    status_t status;
    appid_t app_id;
    unsigned char exec_position;
    peid_t core_idx;
    int force_pe;
};

#define PHOBJ_MIN_PKT_SZ        (sizeof (struct pkt) + 7)

int phobj_topkt(void *x, char **start, char *end)
{
    T obj = (T) x;

    assert(obj && start && end);
    assert(*start <= end);

    if ((end - *start) < PHOBJ_MIN_PKT_SZ) {
        return 0;
    }

    memcpy(*start, (void *) obj, sizeof (struct pkt));
    *start += sizeof (struct pkt);

    if (!str_topkt(obj->appname, start, end)) {
        return 0;
    }

    if (!str_topkt(obj->objname, start, end)) {
        return 0;
    }

    if (!str_topkt(obj->exename, start, end)) {
        return 0;
    }

    if (!Set_topkt(obj->itfs, start, end, phitf_topkt)) {
        return 0;
    }

    if (!Set_topkt(obj->stats, start, end, var_topkt)) {
        return 0;
    }

    if (!Set_topkt(obj->params, start, end, var_topkt)) {
        return 0;
    }

    *start+=2;

    if (!rtcobj_topkt(obj->rtc, start, end)) {
        return 0;
    }

    return 1;
}

void * phobj_newfrompkt(char **start, char *end)
{
    T obj;

    assert(start && end);
    assert(end >= *start);

    if (end - *start < PHOBJ_MIN_PKT_SZ) {
        return NULL;
    }

    NEW(obj);
    if (!obj) {
        return NULL;
    }

    memcpy((void *) obj, *start, sizeof (struct pkt));
    *start += sizeof (struct pkt);

    obj->appname = (str_o) str_newfrompkt(start, end);
    if (!obj->appname) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    obj->objname = (str_o) str_newfrompkt(start, end);
    if (!obj->objname) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    obj->exename = (str_o) str_newfrompkt(start, end);
    if (!obj->exename) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    obj->itfs = Set_newfrompkt(start, end, phitf_newfrompkt);
    if (!obj->itfs) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    obj->stats = Set_newfrompkt(start, end, var_newfrompkt);
    if (!obj->stats) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    obj->params = Set_newfrompkt(start, end, var_newfrompkt);
    if (!obj->params) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    *start+=2;

    obj->rtc = rtcobj_newfrompkt(start, end);
    if (!obj->rtc) {
        phobj_delete((T*) & obj);
        return NULL;
    }

    return (T) obj;
}

int phobj_cmpid(const void *x, const void *member)
{
    T obj = (T) member;
    T tmp = (T) x;

    return tmp->obj_id != obj->obj_id;
}

int phobj_findid(const void *x, const void *member)
{
    T obj = (T) member;

    return *((objid_t *) x) != obj->obj_id;
}

int phobj_findobjname(const void *x, const void *member)
{
    T obj = (T) member;

    return str_cmp(obj->objname, (str_o) x);
}

int phobj_findappname(const void *x, const void *member)
{
    T obj = (T) member;

    return str_cmp(obj->appname, (str_o) x);
}

/** Returns true if status in x matches object's status
 * in member, false otherwise
 */
int phobj_finddiffstatus(const void *x, const void *member)
{
    T obj = (T) member;

    return *((status_t *) x) == obj->status;
}

void phobj_xdelete(void **x)
{
    phobj_delete((T*) x);
}

void * phobj_xdup(const void *x)
{
    return (void *) phobj_dup((T) x);
}
