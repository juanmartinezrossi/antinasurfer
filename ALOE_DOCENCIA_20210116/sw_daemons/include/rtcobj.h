/*
 * rtcobj.h
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
#ifndef RTCOBJ_INCLUDED
#define RTCOBJ_INCLUDED

#define T rtcobj_o

#define rtcobj_sizeof sizeof(struct rtcobj_o)

/** @defgroup rtc RT Object Propierties
 *
 * @ingroup common_obj
 *
 * @todo Define this
 * 
 * RT Object parameters
 */
typedef struct T *T;

struct T {
    float mops_req;
    int tstamp;
    int rep_tstamp;
    int nvcs;
    int start_usec;
    int end_usec;
    int max_end_usec;
    int mean_period;
    int cpu_usec;
    int max_usec;
    float mean_mops;
    float max_mops;
    int nof_faults;
};

T rtcobj_new();
void rtcobj_delete(T * obj);
T rtcobj_dup(T obj);
int rtcobj_topkt(void *x, char **start, char *end);
void * rtcobj_newfrompkt(char **start, char *end);
void rtcobj_xdelete(void **x);
void * rtcobj_xdup(const void *x);
int rtcobj_xsizeof(const void *x);

/** @} */
#undef T  
#endif /*  */
