/*
 * app.c
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

#include "pkt.h"
#include "app.h"
#include "phitf.h"
#include "rtcobj.h"
#include "phobj.h"

#define T app_o

int appid = 1;

struct apptopkt {
    appid_t app_id;
    status_t cur_status;
    int nof_rtfaults;
    int report_tstamp;
};

T app_new()
{
    T app;

    NEW(app);
    if (!app) {
        return NULL;
    }

    app->app_id = (appid_t) appid++;
    app->name = (str_o) str_new();
    app->objects = Set_new(0, NULL);
    app->report_tstamp = 0;

    if (!app->objects || !app->name) {
        app_delete((T*) & app);
        return NULL;
    }

    return app;
}

void app_delete(T *app)
{
    assert(app);
    assert(*app);

    if ((*app)->name) {
        str_delete(&(*app)->name);
    }
    if ((*app)->objects) {
        Set_destroy(&(*app)->objects, phobj_xdelete);
    }

    DELETE(*app);
    app = NULL;
}

T app_dupnoobj(T app)
{
    T newapp;

    newapp = app_new();
    if (!newapp) {
        return NULL;
    }

    memcpy(newapp, app, sizeof (struct apptopkt));
    str_cpy(newapp->name, app->name);

    return newapp;
}

void * app_xdupnoobj (const void * app)
{
    return (void*) app_dupnoobj((T) app);
}

int app_findname(const void *x, const void *member)
{
    T app = (T) member;

    return str_cmp(app->name, (str_o) x);
}

int app_findid(const void *x, const void *member)
{
    T app = (T) member;

    return (*((appid_t *) x) != app->app_id);
}

int app_topkt(void *x, char **start, char *end)
{
    T app = (T) x;

    memcpy(*start, app, sizeof (struct apptopkt));
    *start += sizeof (struct apptopkt);

    if (!str_topkt(app->name, start, end)) {
        return 0;
    }

    if (!Set_topkt(app->objects, start, end, phobj_topkt)) {
        return 0;
    }

    return 1;
}

void * app_newfrompkt(char **start, char *end)
{
    T app;

    assert(start && end);
    assert(end >= *start);

    if (end - *start < sizeof (struct apptopkt)) {
        return NULL;
    }

    NEW(app);
    if (!app) {
        return NULL;
    }

    memcpy(app, *start, sizeof (struct apptopkt));
    *start += sizeof (struct apptopkt);

    app->name = (str_o) str_newfrompkt(start, end);
    if (!app->name) {
        app_delete((T*) & app);
        return NULL;
    }

    app->objects = Set_newfrompkt(start, end, phobj_newfrompkt);
    if (!app->objects) {
        app_delete((T*) & app);
        return NULL;
    }

    return app;
}

void app_xdelete(void **x)
{
    app_delete((T*) x);
}

void app_sdelete(void **x)
{
    assert(x);
    assert(*x);
    DELETE(*x);
    x = NULL;
}

phobj_o app_findobjpkt(app_o app, pkt_o pkt)
{
    phobj_o obj;
    str_o obj_name;

    obj_name = pkt_get(pkt, FIELD_OBJNAME, str_newfrompkt);
    if (!obj_name) {
        return NULL;
    }
    obj = Set_find(app->objects, obj_name, phobj_findobjname);
    str_delete(&obj_name);
    if (!obj) {
        return NULL;
    }
    return obj;
}

phobj_o app_findappobjpkt(Set_o app_set, pkt_o pkt)
{
    str_o app_name;
    app_o app;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        return NULL;
    }
    app = Set_find(app_set, app_name, app_findname);
    str_delete(&app_name);
    if (!app) {
        return NULL;
    }

    return app_findobjpkt(app, pkt);
}

















