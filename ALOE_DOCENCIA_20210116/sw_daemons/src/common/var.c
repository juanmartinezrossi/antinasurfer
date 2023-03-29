/*
 * var.c
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
#include <stdio.h>

#include "phal_hw_api.h"

#include "phid.h"
#include "stats.h"


#include "str.h"
#include "var.h"
#include "log.h"
#include "set.h"
#include "cfg_parser.h"

/* public type */
#define T var_o

/* private type */
#define V var_x

char varBuffer[STATS_MAX_LENGTH];

typedef struct V *V;

struct topkt {
    varid_t stat_id;
    int tstamp;
    varsz_t size;
    vartype_t type;
};

/** Constructor
 *
 * @param size Size of the variable in elements
 * @param type Type of the variable
 *
 * @return !=0 pointer to the object
 * @return =0 Error
 *
 * @todo TEMA SIZES VARIABLES!! ANEM A WORDS O BYTS O QUE??
 * @todo Just Integers supported, type is ignored
 */
T
var_new(varsz_t size, vartype_t type)
{

    V var;

    NEW(var);
    if (!var) {
        return NULL;
    }

    var->maxsize = size;
    var->type = type;
    var->i_stat_id=0;

    var->elem_sz = var_sizeoftype(type);
    if (var->elem_sz < 0) {
        var_delete((T*) & var);
        return NULL;
    }

    if (var->elem_sz * size > STATS_MAX_LENGTH) {
        printf("VAR: Error size exceeds maximum stats size (%d>%d)\n",var->elem_sz*size,STATS_MAX_LENGTH);
        return NULL;
    }


    var->name = str_new();
    if (!var->name) {
        var_delete((T*) & var);
        return NULL;
    }
    return (T) var;
}

int var_sizeoftype(vartype_t type)
{
    int elem_sz;
    switch (type) {
    case STAT_TYPE_TEXT:
    case STAT_TYPE_CHAR:
    case STAT_TYPE_UCHAR:
        elem_sz = sizeof (char);
        break;
    case STAT_TYPE_FLOAT:
        elem_sz = sizeof (float);
        break;
    case STAT_TYPE_INT:
        elem_sz = sizeof (int);
        break;
    case STAT_TYPE_SHORT:
        elem_sz = sizeof (short);
        break;
    default:
        xprintf("Unknown Var type %d\n", type);
        return -1;
    }
    return elem_sz;
}

void var_delete(T * x)
{
    V * var = (V*) x;

    assert(var);
    assert(*var);

    if ((*var)->name) {
        str_delete(&(*var)->name);
    }
    if ((*var)->i_stat_id) {
        hwapi_var_delete((*var)->i_stat_id);
    }

    DELETE(*var);
    var = NULL;
}

vartype_t var_type(T x)
{
    V var = (V) x;
    assert(var);

    return var->type;
}

log_o var_getlog(T x)
{
    V var = (V) x;
    assert(var);
    return var->report;
}

void var_setlog(T x, log_o log)
{
    V var = (V) x;
    assert(var);
    var->report = log;
}

/** Get maximum allocated size in bytes
 */
int var_maxbsize(T x)
{
    V var = (V) x;

    assert(x);

    return var->maxsize * var->elem_sz;
}

/** Get current size in bytes
 */
int var_bsize(T x)
{
    V var = (V) x;

    assert(x);

    return var->size * var->elem_sz;
}

/** Get current size in number of elements
 */
int var_length(T x)
{
    V var = (V) x;

    assert(x);

    return var->size;
}

/** Get maximum allocated size in number of elements
 */
int var_maxlength(T x)
{
    V var = (V) x;

    assert(x);

    return var->maxsize;
}

int var_readvalue(T x, key_o value, int type)
{
    V var = (V) x;
    assert(var);
    var->size = var->maxsize;
    if (!var->i_stat_id) {
        var->i_stat_id = hwapi_var_create(str_str(x->name),var->elem_sz * var->size,0);
        if (!var->i_stat_id) {
            return 0;
        }

    }
    if (!key_value(value, var->size, type, varBuffer)) {
        printf("VAR: Error reading variable.\n");
        return 0;
    }

    if (hwapi_var_write(var->i_stat_id,varBuffer,var->size*var->elem_sz)>0) {
        return 1;
    } else {
        printf("VAR: Error writting to temp variable\n");
        return 0;
    }
}

/** Modify variable value 
 *
 * Modifies the stored variable value with the given value.
 * User must indicate new value size although if it is bigger
 * than the previously defined one, this excess will not be stored.
 *
 * @param var Variable object pointer
 * @param value Pointer to new value contentes
 * @param size Size of the new value to store
 *
 * @return >=0 Number of bytes written
 * @return -1 Error
 *
 */
varsz_t var_setvalue(T x, void *value, varsz_t size)
{
    V var = (V) x;

    assert(var && value);

    if (!size) {
        printf("VAR: Error setting stat value. Size is 0\n");
        return 0;
    }

    /* check sizes */
    if (size > var->maxsize) {
        size = var->maxsize;
    }

    if (!var->i_stat_id) {
        var->i_stat_id = hwapi_var_create(str_str(x->name),var->elem_sz * var->maxsize,0);
        if (!var->i_stat_id) {
            printf("VAR: Error setting stat value. Error creating temp stat\n");
            return 0;
        }

    }

    /* copy data */
    if (hwapi_var_write(var->i_stat_id,value,size * var->elem_sz)<0) {
        printf("VAR: Error setting stat value. Error writting temp stat\n");
        return 0;
    }
    var->size = size;
    return size;
}

/** Get variable value 
 *
 * Obtains the stored variable value address.
 */
int var_getvalue(T x, void *value, int max_size)
{
    V var = (V) x;
    int size;

    assert(var);

    if (!var->i_stat_id) {
        printf("VAR: Error i_var not created\n");
        return 0;
    }
    size = var_bsize(var);
    if (size>max_size) {
        printf("VAR: Caution truncating variable contents\n");
        size=max_size;
    }
    if (hwapi_var_read(var->i_stat_id,value,size)<0) {
        return 0;
    }
    return 1;
}

int var_readreport(T x, void *data, int *tsvec, int idx, int window_sz)
{
    V var = (V) x;
    char *str = (char*) data;
    assert(x && data && tsvec);

    var->size = ((hwapi_var_getsz(var->stat_id) - 1) / var->elem_sz) + 1;
    if (hwapi_var_readts(var->stat_id, &str[idx * var_bsize(x)],
            (window_sz - idx) * var_bsize(x),
            &tsvec[idx]) < 0) {
        return 0;
    }
    return 1;
}

void var_printstr(T x, char *str)
{
    V var = (V) x;

    if (hwapi_var_read(var->i_stat_id,varBuffer,var_bsize(var))<0) {
        return;
    }

    var_printstr_v(var->type, var->size, str, varBuffer);
}

void var_printstr_v(vartype_t type, varsz_t size, char *str, void *value)
{
    int i;
    char comma[] = ", ";
    char space[] = " ";
    char *sep;
    char *data_c;
    float *data_f;
    int *data_i;
    short *data_s;
    int off;

    assert(str);

    if (!value) {
        return;
    }

    if (type == STAT_TYPE_TEXT && size > 1) {
        sprintf(str, "%s", (char *) value);
        return;
    }


    data_c = (char *) value;
    data_f = (float *) value;
    data_i = (int *) value;
    data_s = (short *) value;

    off = 0;
    for (i = 0; i < size; i++) {
        if (i + 1 < size) {
            sep = comma;
        } else {
            sep = space;
        }
        switch (type) {
        case STAT_TYPE_CHAR:
            off += sprintf(&str[off], "%d%s", data_c[i], sep);
            break;
        case STAT_TYPE_UCHAR:
            off += sprintf(&str[off], "%d%s", (unsigned char) data_c[i], sep);
        case STAT_TYPE_FLOAT:
            off += sprintf(&str[off], "%f%s", data_f[i], sep);
            break;
        case STAT_TYPE_INT:
            off += sprintf(&str[off], "%d%s", data_i[i], sep);
            break;
        case STAT_TYPE_SHORT:
            off += sprintf(&str[off], "%d%s", data_s[i], sep);
            break;
        }
    }
}

T var_dup(T x)
{
    V var = (V) x;

    V newvar;

    assert(var);

    newvar = (V) var_new(var->maxsize, var->type);

    newvar->stat_id = var->stat_id;
    newvar->i_stat_id = var->i_stat_id;
    str_cpy(newvar->name, var->name);

    return (void*) newvar;
}

/** Size of object structure when writting to flat array
 *
 */
int var_sizeof_var(T x)
{
    V var = (V) x;

    int size;

    assert(var);

    size = sizeof (struct topkt) + str_len(var->name) + var->size
            * var->elem_sz;

    return size;
}

int var_writetohwapi(T x)
{
    V var = (V) x;

    assert(var);

    if (!var->i_stat_id || !var->size) {
        printf("VAR: Error reading from hwapi. Value not created\n");
        return 0;
    }

    if (hwapi_var_read(var->i_stat_id,varBuffer,var->size * var->elem_sz) < 0) {
        printf("VAR: Error reading from hwapi. Error reading temp stat\n");
        return 0;
    }

    if (hwapi_var_write(var->stat_id, varBuffer, var->size * var->elem_sz)
            > 0) {
        return 1;
    } else {
        return 0;
    }
}

int var_readfromhwapi(T x)
{
    V var = (V) x;
    var->readfromhwapi=1;
}

void * var_newfrompkt(char **start, char *end)
{
    struct topkt pkt;

    V var;

    assert(start && end && (end >= *start));

    memcpy(&pkt,*start,sizeof(struct topkt));

    var = (V) var_new(pkt.size, pkt.type);
    if (!var) {
        return NULL;
    }

    var->stat_id = pkt.stat_id;
    var->tstamp = pkt.tstamp;
    var->size = pkt.size;

    *start += sizeof (struct topkt);

    str_delete(&var->name);
    var->name = str_newfrompkt(start, end);
    if (!var->name) {
        var_delete((T*) & var);
        return NULL;
    }

    /* 1 means there is value, 0 means there isn't */
    if (**start) {
        (*start)++;
        var_setvalue((T) var, *start, var->size);
        *start += var->size * var->elem_sz;
    } else {
        (*start)++;
    }

    return (void *) var;
}

/** Get Variable Raw Data.
 *
 * Copies all structure to a flat array of data. It can be reversed
 * with the newfromdata function
 */
int var_topkt(void *x, char **start, char *end)
{
    V var = (V) x;

    assert(var && start && end);
    assert(end >= *start);

    if ((end - *start) < var_sizeof_var((T) var)) {
        return 0;
    }
    if (!var->size) {
        var->size = var->maxsize;
    }
    memcpy(*start, &var->stat_id, sizeof (struct topkt));
    *start += sizeof (struct topkt);

    if (!str_topkt(var->name, start, end)) {
        return 0;
    }

    /* 1 means there is value, 0 means there isn't */
    if (var->readfromhwapi) {
        **start = 1;
        (*start)++;
        if (hwapi_var_read(var->stat_id,*start,var->size * var->elem_sz) < 0) {
            printf("VAR: Error putting to pkt. Error reading from hwapi\n");
        }
        *start += var->size * var->elem_sz;
    } else if (var->i_stat_id) {
        **start = 1;
        (*start)++;
        if (hwapi_var_read(var->i_stat_id,*start,var->size * var->elem_sz)<0) {
            printf("VAR: Error putting to pkt. Error reading from tmpvar\n");
        }
        *start += var->size * var->elem_sz;
    } else {
        **start = 0;
        (*start)++;
    }

    return 1;
}

int var_findid(const void *x, const void *member)
{
    V var = (V) member;

    return *((varid_t *) x) != var->stat_id;
}

int var_findname(const void *x, const void *member)
{
    V var = (V) member;

    return str_cmp(var->name, (str_o) x);
}

void var_xdelete(void **x)
{
    var_delete((T*) x);
}

int var_xsizeof(const void *x)
{
    return var_sizeof_var((T) x);
}

void * var_xdup(const void *x)
{
    return (void *) var_dup((T) x);
}
