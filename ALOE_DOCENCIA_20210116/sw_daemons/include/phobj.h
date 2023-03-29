/*
 * phobj.h
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
#ifndef PHOBJ_INCLUDED
#define PHOBJ_INCLUDED

#define T phobj_o

#define phobj_sizeof sizeof(struct phobj_o)

/** @defgroup phobj ALOE Object
 * @ingroup common_obj
 *
 * This object is used by several Daemons and is used to define Object in the ALOE environment.
 *
 * It does not implement any complex functionality at all, it is inteded to use it to store object information
 * and access it.
 *
 * @{
 */



typedef struct T *T;

struct T {
    /* public members */
    objid_t obj_id; /**< Id of the objiect */
    peid_t pe_id; /**< Id of the processor where object is running */
    status_t status; /**< Execution status of the object */
    appid_t app_id;
    unsigned short exec_position;
    peid_t core_idx;
    int force_pe;
    str_o appname; /**< Application where object is running */
    str_o objname; /**< Name of the object instance */
    str_o exename; /**< Executable name for the object */
    Set_o itfs; /**< Set of interfaces for the object */
    Set_o stats; /**< Set of stats */
    Set_o params; /**< Initialization Parameters */
    rtcobj_o rtc; /**< RTC Information or requirements */
    Set_o logs; /**< Set of logs */
    
};

T phobj_new(void);
void phobj_delete(T *o);
T phobj_dup(T o);
int phobj_topkt(void *x, char **start, char *end);
void * phobj_newfrompkt(char **start, char *end);
int phobj_findid(const void *x, const void *member);
int phobj_cmpid(const void *x, const void *member);
int phobj_findobjname(const void *x, const void *member);
int phobj_findappname(const void *x, const void *member);
int phobj_finddiffstatus(const void *x, const void *member);
int phobj_xsizeof(const void *x);
void phobj_xdelete(void **x);
void * phobj_xdup(const void *x);

/** @} */
#undef T  
#endif /*  */
