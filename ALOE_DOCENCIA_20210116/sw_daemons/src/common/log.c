/*
 * log.c
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
#include "phal_hw_api_man.h"

#include "phid.h"

#include "str.h"
#include "set.h"
#include "log.h"
#include "var.h"

#define T log_o

T log_new(int id, char *name)
{
    T log;
    int len;

    NEW(log);
    if (!log) {
        return NULL;
    }

    log->fd = hwapi_res_open(name, FLOW_WRITE_ONLY, &len);
    if (log->fd < 0) {
        log_delete(&log);
        return NULL;
    }

    /* set position to the end */
    hwapi_res_read(log->fd, NULL, len);
    log->name = str_snew(name);
    if (!log) {
        log_delete(&log);
        return NULL;
    }

    log->id = id;

    return log;
}

void log_xdelete(void ** x)
{
    log_delete((T*) x);
}

void log_delete(T * log)
{
    assert(log);
    assert(*log);


    if ((*log)->name) {
        str_delete(&(*log)->name);
    }

    if ((*log)->fd > 0) {
        hwapi_res_close((*log)->fd);
    }
    DELETE(*log);
    *log = NULL;
}

T log_dup(T log)
{
    T newlog;

    assert(log);

    NEW(newlog);
    if (!newlog) {
        return NULL;
    }
    newlog->fd = log->fd;
    newlog->id = log->id;
    newlog->name = str_dup(log->name);

    if (!newlog->name) {
        log_delete(&newlog);
        return NULL;
    }

    return newlog;
}

int log_Write(T log, char *output)
{
    assert(log);

    return hwapi_res_write(log->fd, output, strlen(output));
}

/**@todo Use malloc instead of static of char buffer for logging stats reports
 */
char s[10 * 1024];

int log_Write_var(T log, vartype_t type, varsz_t size, void *data, int *ts_vec, int report_len)
{
    int i, n;
    char *str = (char *) data;
    assert(data && ts_vec && report_len >= 0);

    for (i = 0; i < report_len; i++) {
        sprintf(s, "%d:", ts_vec[i]);
        var_printstr_v(type, size, &s[strlen(s)], &str[i * var_sizeoftype(type)]);
        n = strlen(s);
        s[n++] = '\n';
        s[n++] = '\0';

        hwapi_res_write(log->fd, s, strlen(s));
    }
    return 1;
}

int log_findid(const void *x, const void *member)
{
    T log = (T) member;

    return (*((varid_t *) x) != log->id);
}


