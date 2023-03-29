/*
 * var.h
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
#ifndef VAR_INCLUDED
#define VAR_INCLUDED

#include "log.h"

#define T var_o
#define V var_x

struct V {
    str_o name;
    varid_t stat_id;
    int tstamp;
    varsz_t size;
    vartype_t type;
    log_o report;
    int elem_sz;
    varsz_t maxsize;
    varid_t i_stat_id;
    char readfromhwapi;
};

#define var_sizeof sizeof(struct V)


/** @defgroup var ALOE Object Stat/Param
 * @ingroup common_obj
 *
 * This object is used to deal with ALOE variables (statistics or parameters).
 * It allows to access its name, id and value 
 * 
 * @todo Check values methods
 *
 * @{
 */


typedef struct T *T;

struct T {
    str_o name;
    varid_t id;
    int tstamp;
};


T var_new(varsz_t size, vartype_t type);
void var_delete(T * x);
T var_dup(T x);
int var_bsize(T x);
int var_maxbsize(T x);
int var_sizeoftype(vartype_t type);
int var_length(T x);
int var_maxlength(T x);
vartype_t var_type(T x);
log_o var_getlog(T x);
void var_setlog(T x, log_o log);
varsz_t var_setvalue(T x, void *value, varsz_t size);
int var_getvalue(T x, void *value, int max_size);
int var_readreport(T x, void *data, int *tsvec, int idx, int window_sz);
int var_writetohwapi(T x);
int var_readfromhwapi(T x);
void var_printstr(T x, char *str);
void var_printstr_v(vartype_t type, varsz_t size, char *str, void *value);
void * var_newfrompkt(char **start, char *end);
int var_topkt(void *x, char **start, char *end);
int var_findid(const void *x, const void *member);
int var_findname(const void *x, const void *member);
void var_xdelete(void **x);
void * var_xdup(const void *x);
int var_xsizeof(const void *x);


/** @} */
#undef T  
#endif /*  */
