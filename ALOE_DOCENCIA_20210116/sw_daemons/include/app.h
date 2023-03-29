/*
 * app.h
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

#ifndef APP_INCLUDED
#define APP_INCLUDED

#define T app_o

#define app_sizeof sizeof(struct app_o)

/** @defgroup app ALOE Application
 * @ingroup common_obj
 *
 * Manages an application, its execution status and the objects belonging to it.
 *
 * @{
 */

typedef struct T *T;

struct T {
    appid_t app_id;
    status_t cur_status;
    int nof_rtfaults;
    int report_tstamp;
    str_o name;
    Set_o objects;
    status_t next_pending_status;
    int next_pending_tstamp;
};

T app_new();
void app_delete(T *app);
T app_dupnoobj(T app);
int app_findname(const void *x, const void *member);
int app_findid(const void *x, const void *member);
int app_topkt(void *x, char **start, char *end);
void * app_newfrompkt(char **start, char *end);
void app_xdelete(void **x);
void * app_xdupnoobj(const void * app);



/** @} */

#undef T
#endif /*  */
