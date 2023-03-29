/*
 * pe.h
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
#ifndef pe_INCLUDED
#define pe_INCLUDED

#define T pe_o

#define pe_sizeof sizeof(struct pe_o)

/** @defgroup app ALOE Application 
 * @ingroup common_obj
 *
 * Manages an application, its execution status and the objects belonging to it. 
 *
 * @{
 */

typedef struct T *T;

struct T {
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
    str_o name;
    Set_o xitfs;
};

T pe_new();
T pe_dup(T x);
void pe_delete(T *app);
int pe_findid(const void *x, const void *member);
int pe_findcoreid(const void *x, const void *member);
int pe_findPidx(const void *x, const void *member);
int pe_topkt(void *x, char **start, char *end);
void * pe_newfrompkt(char **start, char *end);
void pe_xdelete(void ** x);

/** @} */

#undef T
#endif /*  */
